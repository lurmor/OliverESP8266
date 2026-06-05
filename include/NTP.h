#ifndef NTP_H
#define NTP_H
#include <Arduino.h>
#include <WiFiUdp.h>
#include "globalData.h"
// unsigned long getAveragedTime();
uint64_t GetRealTime();
uint16_t GetRealTimeMarker();
// void ForceNTPUpdate();
bool TrySincTime();
#endif /* NTP_H */
