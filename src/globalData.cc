#include "globalData.h"

SMFlags GlobalSMFlags = 0;
IPAddress serverIP;
uint16_t tcpServerPort = 0;
WiFiClient client;
unsigned long unixTimeShift = 0;
double timeSpeed = 1;