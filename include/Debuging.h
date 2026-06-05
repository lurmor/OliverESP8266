#ifndef DEBUGING_H
#define DEBUGING_H

#include <Arduino.h>
#define DEBUG
#define STAT


void PrintLog(const Printable &str);
template <typename T>
void PrintLog(T str)
{
#ifdef DEBUG
    Serial.print(str);
#endif
};

template <typename T>
void PrintLogln(T str)
{
#ifdef DEBUG
    Serial.println(str);
#endif
};

#endif