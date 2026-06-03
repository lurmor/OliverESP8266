#ifndef SENRRESUDP_H
#define SENRRESUDP_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include "simpleList.h"
class UdpSender
{
private:
    WiFiUDP _udp;
    IPAddress _remoteIP; // TODO List
    uint16_t _remotePort;
    uint16_t _localPort;
    bool _started = false;

public:
    UdpSender(uint16_t remotePort, uint16_t localPort);

    bool CanSend();
    void Begin();
    void SetRemoteIP(IPAddress ipaddr);
    bool Send(const uint8_t *data, size_t len);
    bool SendUint64(uint64_t value);
};

class UdpReceiver
{
private:
    WiFiUDP _udp;
    uint16_t _localPort;
    bool _started = false;

    uint8_t _buffer[2048];
    size_t _packetSize;

public:
    UdpReceiver(uint16_t localPort);

    void Begin();
    int Update();
    const uint8_t *GetData() const;
    size_t GetSize() const;
    uint64_t ReadUint64();
    IPAddress GetRemoteIP();
    uint16_t GetRemotePort();
};

#endif /* SENRRESUDP_H */
