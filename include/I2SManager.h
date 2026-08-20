#ifndef I2SMANAGER_H
#define I2SMANAGER_H

#include <Arduino.h>
#include <driver/i2s.h>

// Перечисление типов устройств, которые мы можем распознать
enum AudioDeviceType
{
    DEVICE_NONE,
    DEVICE_DAC_ONLY,  // Только выход
    DEVICE_ADC_ONLY,  // Только вход
    DEVICE_FULL_CODEC // И вход, и выход
};

class I2SManager
{
private:
    i2s_port_t _i2sPort;
    int _bckPin, _wsPin, _dataPin;
    int _detectOutPin;
    int _detectInPin;
    int _mckPin;
    uint32_t _sampleRate;

    AudioDeviceType _currentDevice;
    bool _isI2SRunning;

    void stopI2SAps(); // Внутренний безопасный стоп железа
    i2s_mode_t getI2SModeByType(AudioDeviceType type);

public:
    I2SManager(i2s_port_t port = I2S_NUM_0);

    // В инициализацию теперь передаем пин детекта
    void init(int mck, int bck, int ws, int dataPin, int detectOutPin, int detectInPin, uint32_t sampleRate = 48000);

    // Функция проверки: опрашивает пин и управляет состоянием драйвера
    AudioDeviceType checkConnection();
    AudioDeviceType getType();
    // Безопасный запуск нужного режима
    bool start(AudioDeviceType type);

    // Безопасная остановка при выдергивании платы
    void stop();
    void clear();

    size_t write(const void *data, size_t size);
    size_t read(void *dest, size_t size);
};

#endif /* I2SMANAGER_H */
