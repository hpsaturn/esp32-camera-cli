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

// Please change these values via CLI. Defaults:
int setup_mode = 0;
bool jpgmode = true;
bool debug = false;
int jpgwait = 60;
int jpgqlty = 9;

void processFrameInternalJPG() {
  if (Camera.get()) {
    radio.sendData(Camera.fb->buf, Camera.fb->len);
    delay(jpgwait);  // ==> weird delay when you are using only DRAM.
    if (debug) printFPS("CAM:");
    Camera.free();
  }
}

void processFrameExternalJPG() {
  if (Camera.get()) {
    uint8_t *out_jpg = NULL;
    size_t out_jpg_len = 0;
    frame2jpg(Camera.fb, Camera.config.jpeg_quality, &out_jpg, &out_jpg_len);
    radio.sendData(out_jpg, out_jpg_len);
    delay(jpgwait);  // ==> weird delay when you are using only DRAM.
    if (debug) printFPS("CAM:");
    free(out_jpg);
    Camera.free();
  }
}

/**
 * @brief config without PSRAM, only DRAM. (faster and better quality)
 */
void configCameraInternalJPG() {
  
  Camera.config.jpeg_quality = jpgqlty;
  Camera.config.frame_size = FRAMESIZE_240X240;
  Camera.config.pixel_format = PIXFORMAT_JPEG;
  Camera.config.fb_location = CAMERA_FB_IN_DRAM;
}

/**
 * @brief Config using PSRAM if it exists and external JPG processing. (improved transfer size)
 */
void configCameraExternalJPG() {
  Camera.config.jpeg_quality = jpgqlty;
  Camera.config.frame_size = FRAMESIZE_240X240;
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
  response->printf("jpeg decoder \t: %s\r\n",jpgmode ? "internal" : "external");
  response->printf("jpeg quality \t: %i\r\n",Camera.config.jpeg_quality);
  response->printf("jpeg delay   \t: %i\r\n",jpgwait);
  response->printf("fbuffer count\t: %i\r\n",Camera.config.fb_count);
  response->printf("frame size   \t: %i\r\n",Camera.config.frame_size);
}

void info(char *args, Stream *response) {
  wcli.status(response);
  printSettings(response);
}

void set_jpgm(char *args, Stream *response) {
  if (!setup_mode){
    response->println("This setting is only possible in setup mode. Type setup");
    return;
  }
  jpgmode = !jpgmode;
  cfg.saveBool(PKEYS::KJPGM, jpgmode);
  if (jpgmode) {
    response->println("processing with internal JPG decoder");
  }
  else {
    response->println("processing with external JPG decoder");
  }
}

void set_jpgq(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  jpgqlty = operands.first().toInt();
  Camera.config.jpeg_quality = jpgqlty;
  cfg.saveInt(PKEYS::KJPGQ,jpgqlty);
}

void set_jpgd(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  jpgwait = operands.first().toInt();
  response->printf("jpg decoding delay: \t%i\r\n", jpgwait);
  cfg.saveInt(PKEYS::KJPGD,jpgwait);
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
  response->println("\r\n\nEntering to setup mode. Restarting..\r\n");
  delay(2000);
  reboot(nullptr,response);
}

void setup_exit(char *args, Stream *response) {
  cfg.saveBool(PKEYS::KSETUP, false);
  setup_mode = -1;
}

void initCameraSetup() {
  jpgmode = cfg.getBool(PKEYS::KJPGM, true);
  jpgwait = cfg.getInt(PKEYS::KJPGD, 60);
  jpgqlty = cfg.getInt(PKEYS::KJPGQ, 12);
  if (cfg.getBool(PKEYS::KSETUP, false)) {
    setup_mode=1;
    Serial.println("\r\nCamera Setup. Type \"exit\" to leave");
    printSettings(&Serial);
    wcli.shell->setBannerText(setup_prompt);
    while (setup_mode>0) wcli.loop();
    wcli.shell->setBannerText(app_prompt);
    setup_mode=0;
    Serial.println("\r\nSettings saved. Booting..\r\n");
  }
  wcli.loop();
  if (jpgmode)
    configCameraInternalJPG();
  else
    configCameraExternalJPG();
}

void initSerialShell(){
  wcli.setSilentMode(true);  // less debug output
  wcli.add("jpgmode", &set_jpgm,       "\tinternal/external JPG decoder toggle");
  wcli.add("jpgqlty", &set_jpgq,       "\tset JPG quality");
  wcli.add("jpgwait", &set_jpgd,       "\tset JPG decoding delay");
  wcli.add("info",    &info,           "\t\tsystem status info");
  wcli.add("debug",   &enable_debug,   "\t\ttoggle debug mode");
  wcli.add("setup",   &setup_init,     "\t\tenter to safe mode or camera setup");
  wcli.add("exit",    &setup_exit,     "\t\tleave setup mode");
  wcli.add("clear",   &clear,          "\t\tclear shell");
  wcli.add("halt",    &halt,           "\t\tperform a ESP32 deep sleep");
  wcli.add("reboot",  &reboot,         "\tperform a ESP32 reboot");
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

  // My M5Core2 receiver B8:F0:09:C6:0E:CC
  // My CrowPanel receiver B0:81:84:74:18:2C
  // const uint8_t macRecv[6] = {0xB8,0xF0,0x09,0xC6,0x0E,0xCC};
  // const uint8_t macRecv[6] = {0xB0,0x81,0x84,0x74,0x18,0x2C};
  // radio.setTarget(macRecv);
  radio.init();
  
  btnB.attachClick([]() { shutdown(); });  
}

void loop() {
  if (jpgmode) processFrameInternalJPG();
  else processFrameExternalJPG();
  btnB.tick();
  wcli.loop();
}
