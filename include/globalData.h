#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

typedef uint SMFlags;
#define SN 1234567890
#define UNITTYPE 1
#define NTP_DELAY_MS 10000
#define UDP_AUDIO_IN_PORT 45521
#define UDP_AUDIO_OUT_PORT 45522

extern SMFlags GlobalSMFlags;  // объявление
extern IPAddress serverIP;     // объявление
extern uint16_t tcpServerPort; // объявление
extern WiFiClient client;
extern unsigned long unixTimeShift;
extern double timeSpeed;

#endif