# ESP32 Camera CLI

ESP32 Camera manager using a simple CLI.

## TODO

- [x] ESPNow camera mode JPG decoder internal/external mode
- [x] JPG compression
- [x] JPGQ, JPGD, JPG mode persistence
- [x] Setup mode / production mode persistence
- [x] Frame size modes
- [ ] ESP TCP/IP stream camera mode
- [ ] ESP TCP/IP stream camera Telnet access (admin)
- [x] ESPNow camera multiple monitors (1:N) (default)
- [x] ESPNow camera set monitor target (1:1)
- [x] ESPNow setChannel 
- [ ] ESPNow XIAO IA v2 detections
- [ ] Camera sensor settings
- [ ] Camera driver load/unload
- [x] XIAO basic power mode support

## CLI

The current status is **working in progress**, but for now it has these commands working:

```bash
---- Available commands ----

channel:        set the WiFi channel
clear:          clear shell
debug:          toggle debug mode
exit:           leave setup mode
fsize:          set frame size output
halt:           perform a ESP32 deep sleep
info:           system status info
jpgmode:        internal/external JPG decoder toggle
jpgqlty:        set JPG quality
jpgwait:        set JPG decoding delay
reboot:         perform a ESP32 reboot
setup:          enter to safe mode or camera setup
target:         set the macaddress of the display target
```

info command sample:

```bash
camcli:$ info

WiFi SSID       : []
IP address      : 0.0.0.0
RSSI signal     : 000
MAC Address     : 74:XX:XX:XX:XX:FC
Hostname        : esp32s3-XXXXX
Auto connect    : enable
Telnet status   : disable
Memory free     : 200Kb

display target  : FF:FF:FF:FF:FF:FF
frame size      : QVGA     320x240  
jpeg decoder    : internal
jpeg quality    : 30
jpeg delay      : 120
fbuffer count   : 2
channel         : 1
```

## Hardware tested

- [x] XIAO Sense Camera
- [ ] M5CoreS3 Camera
- [ ] TJournal Camera
- [ ] Freenove S3 Camera
- [ ] Others
