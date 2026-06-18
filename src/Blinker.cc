#include "Blinker.h"
#include <Arduino.h>

Blinker::Blinker(int pin, unsigned long interval)
{
    _pin = pin;
    _interval = interval;
    _lastToggleTime = 0;
    _ledState = LOW;
    _isInitialized = false;
}

// Реализация основного метода
void Blinker::update()
{
    if (!_isInitialized)
    {
        pinMode(_pin, OUTPUT);
        _isInitialized = true;
    }

    unsigned long currentMillis = millis();

    if (currentMillis - _lastToggleTime >= _interval)
    {
        _lastToggleTime = currentMillis;
        _ledState = !_ledState;
        digitalWrite(_pin, _ledState);
    }
}

// Реализация перегруженного метода
void Blinker::update(unsigned long newInterval)
{
    _interval = newInterval;
    update(); // Вызываем базовый метод
}