#include <Arduino.h>

static uint32_t frame_camera = 0;
static uint_fast64_t time_stamp_camera = 0;

static void print_FPS(int x, int y, const char *msg, uint32_t &frame, uint_fast64_t &time_stamp, uint32_t len) {
  frame++;
  if (millis() - time_stamp > 1000) {
    time_stamp = millis();
    char output[40];
    sprintf(output, "%s%2d FPS  JPG: %05d\r\n",msg, frame, len);
    frame = 0;
    Serial.print(output);
  } 
}

int str2mac(const char* mac, uint8_t* values){
    if( 6 == sscanf( mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",&values[0], &values[1], &values[2],&values[3], &values[4], &values[5] ) ){
        return 1;
    }else{
        return 0;
    }
}

inline void formatMacAddress(uint8_t *macAddr, char *buffer, int maxLength) {
    snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

inline void printMacAddress(uint8_t * macAddress){
    char macStr[18];
    formatMacAddress(macAddress, macStr, 18);
    log_i("%s",macStr);
}
