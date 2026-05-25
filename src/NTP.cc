#include "NTP.h"
#include "Debuging.h"
const int NTP_PACKET_SIZE = 48;
byte buf[8];

unsigned long lastSync = 0;
unsigned long syncInterval = 2000; // каждые 2 сек

unsigned long acc = 0;
uint32_t start = 0;
bool waiting = false;
bool forceUpdate = false;

unsigned long t0 = 0;

WiFiUDP udp;

void PushTimeQuery()
{
    if (udp.localPort() != 2390)
        udp.begin(2390);

    udp.beginPacket(serverIP, 1230);
    udp.write("T", 1);
    udp.endPacket();
}

unsigned long TryGetTimeResponse()
{
    unsigned long long ms = 0;
    int len = udp.parsePacket();
    while (len == 8)
    {
        udp.read(buf, 8);
        // waiting = false;
        len = udp.parsePacket();
        if (len == 0)
            for (int i = 0; i < 8; i++)
                ms = (ms << 8) | buf[i];
    }
    return ms;
}
void ClearUdp()
{
    int len = -1;
    while (len != 0)
    {
        len = udp.parsePacket();
    }
}

bool TrySincTime()
{

    static unsigned long lastSync = GetRealTime();

    if (GetRealTime() - lastSync < NTP_DELAY_MS && !waiting)
        return false;

    if (GetRealTime() - lastSync > 2 * NTP_DELAY_MS && waiting)
    {
        waiting = false;
        lastSync = GetRealTime();
    }

    if (!waiting)
    {
        ClearUdp();
        t0 = micros();
        PushTimeQuery();
        waiting = true;
    }

    unsigned long t = TryGetTimeResponse();
    if (t != 0)
    {
        // Serial.printf("Synced: %lu\n", t);
        waiting = false;
        unsigned long t1 = micros();
        unsigned long latency = (t1 - t0) / 2;

        if (acc == 0)
            acc = latency;

        acc = (acc + latency) / 2;
        Serial.printf("latency: %lu microSec\n", acc);
        // forceUpdate = false;
        unsigned long ct = t + acc / 1000;
        PrintLogln(ct);
        long error = GetRealTime() - ct;
        if (unixTimeShift == 0)
        {
            unixTimeShift = ct - millis();
            error = 0;
        }

        unixTimeShift = unixTimeShift / 2 + (ct - millis()) / 2;
        PrintLog("Time error :");
        PrintLogln(error);
        if (error > 100 || error < -100)
        {
            waiting = false;
            acc = 0;
            PrintLogln("Time error > 100 !!!!!!!!!!!!!!!!!!!!");
            if (error > 10000 || error < -10000)
            {
                unixTimeShift = 0;
                error = 0;
            }
        }
        else
        {
            double timeSpeedError = (double)error / millis();
            PrintLog("Time Speed error :");
            PrintLogln(timeSpeedError * 1000000);
            timeSpeed = timeSpeed - timeSpeedError / 2;
            lastSync = GetRealTime();
            return true;
        }
    }

    return false;
}

unsigned long
GetRealTime()
{
    // if (unixTimeShift == 0)
    //     return 0;
    return unixTimeShift + millis() * timeSpeed;
}

// void ForceNTPUpdate()
// {
//     waiting = false;
// }
