/**************************************************
 * ESPCam CLI Tester
 * @hpsaturn Copyright (C) 2024
 * ----------------------------
**************************************************/

#include <Arduino.h>
#include <OneButton.h>
#include <ESPNowCam.h>
#include <ESP32WifiCLI.hpp>
#include <drivers/CamXiao.h>
#include <Utils.h>

CamXiao Camera;  
ESPNowCam radio;
OneButton btnB(GPIO_NUM_0, true);

void processFrame() {
  if (Camera.get()) {
    uint8_t *out_jpg = NULL;
    size_t out_jpg_len = 0;
    frame2jpg(Camera.fb, 12, &out_jpg, &out_jpg_len);
    radio.sendData(out_jpg, out_jpg_len);
    // printFPS("CAM:");
    free(out_jpg);
    Camera.free();
  }
}

void shutdown() {
  Serial.println("shutdown..");
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0,0);
  delay(1000);
  esp_deep_sleep_start();
}

void info(char *args, Stream *response) {
  wcli.status(response);
}

void reboot(char *args, Stream *response){
  wcli.shell->clear();
  wcli.client->stop();
  ESP.restart();
}

void initRemoteShell(){
  // if (wcli.isTelnetRunning()) wcli.shellTelnet->attachLogo(logo);
}

void initSerialShell(){
  wcli.setSilentMode(true);  // less debug output
  wcli.add("info", &info,       "\t\tsystem status info");
  wcli.add("reboot", &reboot,   "\tperform a ESP32 reboot");
  // wcli.shell->attachLogo(logo);
  wcli.begin();
}

void setup() {
  Serial.begin(115200);
  delay(1000); // only for debugging 
  
  if(psramFound()){
    size_t psram_size = esp_spiram_get_size() / 1048576;
    Serial.printf("PSRAM size: %dMb\r\n", psram_size);
  }
  
  initSerialShell();
  initRemoteShell();

  if (!Camera.begin()) {
    Serial.println("Camera Init Fail");
    delay(1000);
  }

  // Optional M5Core2 receiver B8:F0:09:C6:0E:CC
  const uint8_t macRecv[6] = {0xB8,0xF0,0x09,0xC6,0x0E,0xCC};
  radio.setTarget(macRecv);
  radio.init();
  
  btnB.attachClick([]() { shutdown(); });  
}

void loop() {
  processFrame();
  btnB.tick();
  wcli.loop();
}
