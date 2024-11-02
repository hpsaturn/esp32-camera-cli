/**************************************************
 * ESP32 Camera CLI
 * @hpsaturn Copyright (C) 2024
 * ----------------------------
 * https://github.com/hpsaturn/esp32-camera-cli
**************************************************/

#include <Arduino.h>
#include <OneButton.h>
#include <ESPNowCam.h>
#include <ESP32WifiCLI.hpp>
#include <EasyPreferences.hpp> 
#include <drivers/CamXiao.h>
#include <Utils.h>

CamXiao Camera;  
ESPNowCam radio;
OneButton btnB(GPIO_NUM_0, true);

const char* app_prompt   = "camcli";
const char* setup_prompt = "setup";

int setup_mode = 0;
bool internal_jpg = true;
bool debug = false;

void processFrameNoJPG() {
  if (Camera.get()) {
    radio.sendData(Camera.fb->buf, Camera.fb->len);
    delay(50);  // ==> weird delay when you are using only DRAM.
    if (debug) printFPS("CAM:");
    Camera.free();
  }
}

void processFrameJPG() {
  if (Camera.get()) {
    uint8_t *out_jpg = NULL;
    size_t out_jpg_len = 0;
    frame2jpg(Camera.fb, Camera.config.jpeg_quality, &out_jpg, &out_jpg_len);
    radio.sendData(out_jpg, out_jpg_len);
    if (debug) printFPS("CAM:");
    free(out_jpg);
    Camera.free();
  }
}

void configCameraNoJPG() {
  // Configuration without using the PSRAM, only DRAM.
  // (faster and better quality)
  Camera.config.pixel_format = PIXFORMAT_JPEG;
  Camera.config.fb_location = CAMERA_FB_IN_DRAM;
}

void configCameraJPG() {
  // Configuration using the PSRAM, external JPG processing.
  // (less RAM and improved transfer size)
  Camera.config.pixel_format = PIXFORMAT_RGB565;
  Camera.config.fb_location = CAMERA_FB_IN_PSRAM;
}

void camera_deep_sleep(bool enable) {
  sensor_t* sensor = esp_camera_sensor_get();
  sensor->set_reg(sensor, 0x3008, 0x40, enable ? 0x40 : 0x00);
}

void cameraInit() {
  if (!Camera.begin()) {
    Serial.println("Camera Init Fail");
    delay(1000);
  }
}

void printSettings(Stream *response){
  response->println();
  response->printf("jpeg internal\t: %s\r\n",internal_jpg ? "enable" : "disable");
  response->printf("jpeg quality \t: %i\r\n",Camera.config.jpeg_quality);
  response->printf("fbuffer count\t: %i\r\n",Camera.config.fb_count);
  response->printf("frame size   \t: %i\r\n",Camera.config.frame_size);
}

void info(char *args, Stream *response) {
  wcli.status(response);
  printSettings(response);
}

void jpgmode(char *args, Stream *response) {
  if (!setup_mode){
    response->println("This setting only is possible in setup mode");
    return;
  }
  internal_jpg = !internal_jpg; 
  if (internal_jpg) {
    response->println("Internal JPG ON");
  }
  else {
    response->println("Internal JPG OFF");
  }
}

void set_jpgq(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  Camera.config.jpeg_quality = operands.first().toInt();
}

void enable_debug(char *args, Stream *response) {
  debug = !debug;
  response->printf("debug mode %s\r\n", debug ? "enable" : "disable");
}

void wcliStop(){
  wcli.shell->clear();
  wcli.client->stop();
}

void shutdown() {
  Serial.println("Shutdown..");
  // camera_deep_sleep(true);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0,0);
  delay(500);
  esp_deep_sleep_start();
}

void reboot(char *args, Stream *response){ 
  wcliStop();
  ESP.restart();
}

void halt(char *args, Stream *response){
  wcliStop();
  shutdown();
}

void clear(char *args, Stream *response){
  wcli.shell->clear();
}

void setup_init(char *args, Stream *response) {
  cfg.saveBool(PKEYS::KSETUP, true);

  response->println("\r\nEntering to setup mode. Restarting..\r\n");
  delay(1000);
  reboot(nullptr,response);
}

void setup_exit(char *args, Stream *response) {
  cfg.saveBool(PKEYS::KSETUP, false);
  setup_mode = -1;
}

void initCameraSetup() {
  if (cfg.getBool(PKEYS::KSETUP, false)) {
    setup_mode=1;
    uint32_t start = millis();
    Serial.println("\r\nCamera Setup. Type \"exit\" to leave");
    printSettings(&Serial);
    wcli.shell->setBannerText(setup_prompt);
    while (setup_mode>0) wcli.loop();
    wcli.shell->setBannerText(app_prompt);
    setup_mode=0;
    Serial.println("\r\nSettings saved. Booting..\r\n");
  }
  wcli.loop();
  if (internal_jpg)
    configCameraNoJPG();
  else
    configCameraJPG();
}

void initSerialShell(){
  wcli.setSilentMode(true);  // less debug output
  wcli.add("jpgmode",&jpgmode,        "\tinternal JPG mode");
  wcli.add("jpgqlty",&set_jpgq,       "\tset JPG quality");
  wcli.add("info",   &info,           "\t\tsystem status info");
  wcli.add("debug",  &enable_debug,   "\t\ttoggle debug mode");
  wcli.add("setup",  &setup_init,     "\t\tenter to safe mode camera setup");
  wcli.add("exit",   &setup_exit,     "\t\tleave setup mode");
  wcli.add("clear",  &clear,          "\t\tclear shell");
  wcli.add("halt",   &halt,           "\t\tperform a ESP32 deep sleep");
  wcli.add("reboot", &reboot,         "\tperform a ESP32 reboot");
  // wcli.shell->attachLogo(logo);
  wcli.begin(app_prompt);
}

void initRemoteShell(){
  // if (wcli.isTelnetRunning()) wcli.shellTelnet->attachLogo(logo);
}

void setup() {
  Serial.begin(115200);
  delay(1000); // only for debugging 

  cfg.init("espcamcli");
  
  if(psramFound()){
    size_t psram_size = esp_spiram_get_size() / 1048576;
    Serial.printf("PSRAM size: %dMb\r\n", psram_size);
  }
  
  initSerialShell();
  initRemoteShell();
  initCameraSetup();
  cameraInit();

  // Optional M5Core2 receiver B8:F0:09:C6:0E:CC
  const uint8_t macRecv[6] = {0xB8,0xF0,0x09,0xC6,0x0E,0xCC};
  radio.setTarget(macRecv);
  radio.init();
  
  btnB.attachClick([]() { shutdown(); });  
}

void loop() {
  if (internal_jpg) processFrameNoJPG();
  else processFrameJPG();
  btnB.tick();
  wcli.loop();
}
