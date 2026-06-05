#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error "Неизвестная платформа! Выберите ESP8266 или ESP32."
#endif
typedef uint SMFlags;
// #define SN 1234567890
#define UNITTYPE 1
#define NTP_DELAY_MS 1000
#define UDP_AUDIO_IN_PORT 45521
#define UDP_AUDIO_OUT_PORT 45522

extern SMFlags GlobalSMFlags;  // объявление
extern IPAddress serverIP;     // объявление
extern uint16_t tcpServerPort; // объявление
extern WiFiClient client;
extern uint64_t unixTimeShift;
extern double timeSpeed;
extern uint32_t SN;

#endif