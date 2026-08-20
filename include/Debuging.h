
#ifndef DEBUGING_H
#define DEBUGING_H

#include <Arduino.h>
#include <stdarg.h>
#include "globalData.h" // Необходимо для работы с переменным числом аргументов (...)

// #define DEBUG
#define WEB_DEBUG
#define STAT

template <typename T>
void PrintLog(T str)
{
    // Принудительно формируем строку до макроса проверки
    String outStr = String(str);
#ifdef DEBUG
    Serial.print(outStr);
#endif
#ifdef WEB_DEBUG
    if (client.connected())
    {
        client.print("L-" + String(SN) + "-" + outStr);
    }
#endif
}

template <typename T>
void PrintLogln(T str)
{
    // Принудительно формируем строку до макроса проверки
    String outStr = String(str);
    PrintLog(outStr);
}

inline void PrintLogf(const char *format, ...)
{
    char buffer[512]; // Буфер на 256 символов. Для ESP32 можно поднять до 512, если логи длинные.
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    String outStr = String(buffer);
    PrintLog(outStr);
}

inline void PrintLog(const Printable &str)
{
#ifdef DEBUG
    Serial.print(str);
#endif
#ifdef WEB_DEBUG
    if (client.connected())
    {
        client.print("L-" + String(SN) + "-");
        client.println(str);
    }
#endif
}

#endif