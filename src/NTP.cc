#include "NTP.h"
#include "Debuging.h"
#include "CircularArray.h"
// Убедись, что класс SlideWindow подключен
// #include "SlideWindow.h"

const int NTP_PACKET_SIZE = 48;
byte buf[8];

uint64_t lastSync = 0;
unsigned long syncInterval = 2000; // каждые 2 сек

// unsigned long acc = 0;
uint32_t start = 0;
bool waiting = false;
bool forceUpdate = false;
bool isUdpInitialized = false;

unsigned long t0 = 0;
WiFiUDP udp;

// Инициализация оконного фильтра для задержки (размер окна = 5)
SlideWindow<uint64_t> ShiftWindow(10);
SlideWindow<double> SpeedWindow(10);

void PushTimeQuery()
{
    if (!isUdpInitialized)
    {
        if (udp.begin(2390))
            isUdpInitialized = true;
    }
    udp.beginPacket(serverIP, 1230);
    udp.write((const uint8_t *)"T", 1);
    udp.endPacket();
}

uint64_t TryGetTimeResponse()
{
    uint64_t ms = 0;
    bool gotData = false;
    int len = udp.parsePacket();

    while (len > 0)
    {
        if (len == 8)
        {
            udp.read(buf, 8);
            gotData = true;
        }
        else
        {
            // Очищаем мусорные пакеты
            while (udp.available())
            {
                udp.read();
            }
        }
        len = udp.parsePacket();
    }

    if (gotData)
    {
        for (int i = 0; i < 8; i++)
            ms = (ms << 8) | buf[i];
    }

    return ms;
}

void ClearUdp()
{
    while (udp.parsePacket() > 0)
    {
        while (udp.available())
        {
            udp.read();
        }
    }
}

bool TrySincTime()
{
    uint64_t currentTime = GetRealTime();

    if (currentTime - lastSync < NTP_DELAY_MS && !waiting)
        return false;

    if (currentTime - lastSync > 2 * NTP_DELAY_MS && waiting)
    {
        waiting = false;
        lastSync = currentTime;
    }

    if (!waiting)
    {
        ClearUdp();
        t0 = micros();
        PushTimeQuery();
        waiting = true;
    }

    uint64_t t = TryGetTimeResponse();
    if (t != 0)
    {
        waiting = false;
        unsigned long t1 = micros();

        unsigned long latency = (unsigned long)(t1 - t0) / 2;
        uint64_t ct = t + latency / 1000;
        long error = (long)(currentTime - ct);

        if (unixTimeShift == 0)
        {
            error = 0;
        }
        ShiftWindow.push((ct - millis()));
        unixTimeShift = ShiftWindow.getAverage();

#ifdef STAT
        PrintLogf("latency: %lu microSec\n", latency);
        PrintLogf("Time error = %d\n", error);
#endif
        if (error > 100 || error < -100)
        {
            PrintLogln("Time error > 100 !!!!!!!!!!!!!!!!!!!!");
            if (error > 10000 || error < -10000)
            {
                ShiftWindow.clear();
                unixTimeShift = 0;
                error = 0;
            }
        }
        else
        {
            uint32_t currentMillis = millis();
            if (currentMillis > 0)
            {
                double timeSpeedError = (double)error / currentMillis;

#ifdef STAT
                PrintLogf("Time Speed error = %d\n", timeSpeedError * 1000000);
#endif
                SpeedWindow.push(timeSpeed - timeSpeedError);
                timeSpeed = SpeedWindow.getAverage();
            }
            lastSync = currentTime;
            return true;
        }
    }

    return false;
}

uint64_t GetRealTime()
{
    return unixTimeShift + (uint64_t)(millis() * timeSpeed);
}

uint16_t GetRealTimeMarker()
{
    return (uint16_t)(GetRealTime() & 0xFFFF);
}