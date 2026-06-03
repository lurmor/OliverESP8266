#include "SenrResUDP.h"
#include "Debuging.h"

UdpSender::UdpSender(uint16_t remotePort, uint16_t localPort)
    : _remotePort(remotePort),
      _localPort(localPort)
{
    Begin();
}

void UdpSender::Begin()
{
    // if (!_started)
    // {
    _udp.begin(_localPort);
    //}
}

void UdpSender::SetRemoteIP(IPAddress ipaddr)
{
    _remoteIP = ipaddr;
    _started = true;
    PrintLogln("Transive remoteIp set to" + _remoteIP.toString());
}

bool UdpSender::Send(const uint8_t *data, size_t len)
{
    if (!_started)
        return false;
    if (!_udp.beginPacket(_remoteIP, _remotePort))
        return false;

    _udp.write(data, len);
    bool res = _udp.endPacket();

    return res;
}

bool UdpSender::SendUint64(uint64_t value)
{
    uint8_t buf[8];
    for (int i = 0; i < 8; i++)
        buf[7 - i] = (value >> (i * 8)) & 0xFF; // big endian

    return Send(buf, 8);
}

UdpReceiver::UdpReceiver(uint16_t localPort)
    : _localPort(localPort)
{
    Begin();
}

void UdpReceiver::Begin()
{
    if (!_started)
    {
        _udp.begin(_localPort);
        _started = true;
    }
}

// Неблокирующая проверка
int UdpReceiver::Update()
{
    if (_started)
    {
        int len = _udp.parsePacket();
        // PrintLogln(len);
        if (len > 0 && len <= sizeof(_buffer) - 1)
        {
            _packetSize = _udp.read(_buffer, len);
            _buffer[len] = '\0';
            return len;
        }
    }

    return 0;
}

const uint8_t *UdpReceiver::GetData() const
{
    return _buffer;
}

size_t UdpReceiver::GetSize() const
{
    return _packetSize;
}

uint64_t UdpReceiver::ReadUint64()
{
    if (_packetSize != 8)
        return 0;

    uint64_t value = 0;
    for (int i = 0; i < 8; i++)
        value = (value << 8) | _buffer[i];

    return value;
}

IPAddress UdpReceiver::GetRemoteIP()
{
    return _udp.remoteIP();
}

uint16_t UdpReceiver::GetRemotePort()
{
    return _udp.remotePort();
}
