#ifndef SENRRESUDP_H
#define SENRRESUDP_H

#include <Arduino.h>
#include <WiFiUdp.h>

class UdpSender
{
private:
    WiFiUDP _udp;
    IPAddress _remoteIP;
    uint16_t _remotePort;
    uint16_t _localPort;
    bool _started;

public:
    UdpSender(IPAddress remoteIP, uint16_t remotePort, uint16_t localPort);

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
    bool _started;

    uint8_t _buffer[256];
    size_t _packetSize;

public:
    UdpReceiver(uint16_t localPort);

    void Begin();

    // неблокирующая проверка
    bool Update();

    const uint8_t *GetData() const;

    size_t GetSize() const;

    uint64_t ReadUint64();

    IPAddress GetRemoteIP();

    uint16_t GetRemotePort();
};

#endif /* SENRRESUDP_H */
