#ifndef NTP_H
#define NTP_H
#include <Arduino.h>
#include <WiFiUdp.h>
#include "globalData.h"
unsigned long getAveragedTime();
unsigned long GetRealTime();
void ForceNTPUpdate();
#endif /* NTP_H */
