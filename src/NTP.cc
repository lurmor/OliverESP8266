#include "NTP.h"
#include "Debuging.h"
const int NTP_PACKET_SIZE = 48;
byte buf[8];

unsigned long lastSync = 0;
unsigned long syncInterval = 2000; // каждые 2 сек

unsigned long acc = 0;
uint32_t start = 0;
bool waiting = false;

unsigned long t0 = 0;

unsigned long getRawNtpTime()
{
    static WiFiUDP udp;
    if (udp.localPort() != 2390)
        udp.begin(2390);

    if (!waiting)
    {
        // Отправка пакета
        udp.beginPacket(serverIP, 1230);
        udp.write("T", 1);
        udp.endPacket();
        waiting = true;
    }

    int len = udp.parsePacket();
    if (len == 8)
    {
        udp.read(buf, 8);
        // waiting = false;
        unsigned long long ms = 0;
        for (int i = 0; i < 8; i++)
            ms = (ms << 8) | buf[i];

        return ms;
    }

    return 0;
}

unsigned long getAveragedTime()
{
    if (!waiting || t0 == 0)
    {
        t0 = micros();
    }

    unsigned long t = getRawNtpTime();
    if (t != 0)
    {
        // Serial.printf("Synced: %lu\n", t);

        unsigned long t1 = micros();
        unsigned long latency = (t1 - t0) / 2;
        if (latency < 1000 * 1000)
        {
            if (acc == 0)
                acc = latency;
            acc = (acc + latency) / 2;
            Serial.printf("latency: %lu\n", acc);
            return t + acc / 1000;
        }
    }

    if (micros() - t0 > NTP_DELAY_MS * 1000)
    {
        waiting = false;
    }
    return 0;
}

unsigned long GetRealTime()
{
    return unixTimeShift + millis() * timeSpeed;
}

void ForceNTPUpdate()
{
    waiting = false;
}
