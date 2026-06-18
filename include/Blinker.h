#ifndef BLINKER_H
#define BLINKER_H

class Blinker
{
private:
    int _pin;
    unsigned long _interval;
    unsigned long _lastToggleTime;
    bool _ledState;
    bool _isInitialized;

public:
    // Конструктор
    Blinker(int pin, unsigned long interval);

    // Основной метод обновления
    void update();

    // Перегруженный метод для изменения интервала на лету
    void update(unsigned long newInterval);
};

#endif /* BLINKER_H */
