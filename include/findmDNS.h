#pragma once
#include <Arduino.h>
#include <WiFiUdp.h>

int findmDNS(IPAddress localIP, String target, IPAddress &ip, uint16_t &port);