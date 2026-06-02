// COPY THIS FILE TO  src/brain/secrets.h  AND FILL IN YOUR VALUES.
// secrets.h is gitignored so credentials don't leak.

#pragma once

// 2.4 GHz network only - the ESP32-S3 on the UNO R4 WiFi does not support 5 GHz.
#define WIFI_SSID     "DannyWiFi_RPT"
#define WIFI_PASSWORD "27021983Ad#!"

// mDNS hostname - reachable at http://<this>.local from phones/laptops
#define ROBOT_HOSTNAME "robo"

// URL of the ESP32-CAM stream (camera advertises itself as robo-cam.local).
// Embedded in the control page as the video source.
#define CAMERA_STREAM_URL "http://robo-cam.local:81/stream"
