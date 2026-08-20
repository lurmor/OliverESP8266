#include "I2SManager.h"
#include "Debuging.h"

I2SManager::I2SManager(i2s_port_t port)
{
    _i2sPort = port;
    _currentDevice = DEVICE_NONE;
    _isI2SRunning = false;
}

void I2SManager::init(int mck, int bck, int ws, int dataPin, int detectOutPin, int detectInPin, uint32_t sampleRate)
{
    _bckPin = bck;
    _wsPin = ws;
    _mckPin = mck;
    _dataPin = dataPin;
    //_dataInPin = dataIn;
    _detectOutPin = detectOutPin;
    _detectInPin = detectInPin;
    _sampleRate = sampleRate;

    // Настраиваем пин детекта как вход с подтяжкой к 3.3В
    if (_detectOutPin >= 0)
        pinMode(_detectOutPin, INPUT_PULLUP);
    if (_detectInPin >= 0)
        pinMode(_detectInPin, INPUT_PULLUP);
}

AudioDeviceType I2SManager::checkConnection()
{
    // Читаем физическое состояние пина
    bool outPresent = (_detectOutPin >= 0) ? (digitalRead(_detectOutPin) == LOW) : false;
    bool inPresent = (_detectInPin >= 0) ? (digitalRead(_detectInPin) == LOW) : false;

    // Определяем текущую аппаратную конфигурацию
    AudioDeviceType detectedDevice = DEVICE_NONE;
    if (outPresent && inPresent)
        detectedDevice = DEVICE_FULL_CODEC;
    else if (outPresent)
        detectedDevice = DEVICE_DAC_ONLY;
    else if (inPresent)
        detectedDevice = DEVICE_ADC_ONLY;

    // Если конфигурация железа изменилась — перестраиваем I2S на лету
    if (detectedDevice != _currentDevice)
    {
        PrintLogf("[I2S] Конфигурация изменилась! Было: %d, Стало: %d\n", _currentDevice, detectedDevice);

        // 1. Всегда тушим старый драйвер, чтобы освободить DMA и прерывания
        if (_isI2SRunning)
        {
            stop();
        }

        _currentDevice = detectedDevice;

        // 2. Если что-то подключено — запускаем I2S в новом правильном режиме
        if (_currentDevice != DEVICE_NONE)
        {
            // i2s_mode_t newMode = getI2SModeByType(_currentDevice);
            if (start(_currentDevice))
            {
                PrintLogln("[I2S] Драйвер успешно переконфигурирован");
            }
            else
            {
                PrintLogln("[I2S] Ошибка переконфигурации!");
            }
        }
    }

    return _currentDevice;
}

AudioDeviceType I2SManager::getType()
{
    return _currentDevice;
}

// i2s_mode_t I2SManager::getI2SModeByType(AudioDeviceType type)
// {
//     switch (type)
//     {
//     case DEVICE_DAC_ONLY:
//         return (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
//     case DEVICE_ADC_ONLY:
//         return (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
//     case DEVICE_FULL_CODEC:
//         return (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX);
//     default:
//         return I2S_MODE_MASTER;
//     }
// }

bool I2SManager::start(AudioDeviceType type)
{
    if (_isI2SRunning)
        return true;
    if (type == DEVICE_ADC_ONLY)
    {

        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = 48000,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
            // .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_MSB,
            .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 10,
            .dma_buf_len = 32,
            .use_apll = true};

        if (i2s_driver_install(_i2sPort, &i2s_config, 0, NULL) != ESP_OK)
            return false;

        i2s_pin_config_t pin_config = {
            .mck_io_num = _mckPin,
            .bck_io_num = _bckPin,
            .ws_io_num = _wsPin,
            .data_out_num = I2S_PIN_NO_CHANGE,
            .data_in_num = _dataPin};

        if (i2s_set_pin(_i2sPort, &pin_config) != ESP_OK)
            return false;

        _isI2SRunning = true;
        return true;
    }
    if (type == DEVICE_DAC_ONLY)
    {

        i2s_config_t i2s_config = {
            // ИСПРАВЛЕНО: Меняем RX на TX, так как мы передаем звук В ЦАП
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
            .sample_rate = _sampleRate,

            // ИСПРАВЛЕНО: Ставим 16 бит, как ты и хотел
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,

            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,

            // ИСПРАВЛЕНО: Для ESP-IDF стандартный формат I2S пишется так.
            // PCM5102 по умолчанию настроен на чистый Philips I2S стандарт.
            .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_MSB,

            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 3,

            // КОРРЕКЦИЯ: Для 16-бит стерео один сэмпл весит 4 байта (2 левый + 2 правый).
            // Размер буфера в байтах должен быть кратен 4. 700 делится на 4 (175 сэмплов),
            // но для DMA стабильнее использовать степени двойки, например 512 или 1024.
            .dma_buf_len = 700,

            .use_apll = false};

        if (i2s_driver_install(_i2sPort, &i2s_config, 0, NULL) != ESP_OK)
            return false;

        i2s_pin_config_t pin_config = {
            .mck_io_num = _mckPin, // Для PCM5102 пин MCK можно не подключать, если на плате замкнута подтяжка к GND (чип сам сгенерирует SCK из BCK)
            .bck_io_num = _bckPin,
            .ws_io_num = _wsPin,

            // ИСПРАВЛЕНО: Направляем данные на ВЫХОД (data_out), а вход отключаем
            .data_out_num = _dataPin,
            .data_in_num = I2S_PIN_NO_CHANGE};

        if (i2s_set_pin(_i2sPort, &pin_config) != ESP_OK)
            return false;

        _isI2SRunning = true;
        return true;
    }
    return false;
}

void I2SManager::stop()
{
    if (!_isI2SRunning)
        return;
    i2s_stop(_i2sPort);
    i2s_driver_uninstall(_i2sPort);
    _isI2SRunning = false;
    PrintLogln("[I2S] Драйвер остановлен и выгружен");
}

void I2SManager::clear()
{
    if (_isI2SRunning)
    {
        i2s_zero_dma_buffer(_i2sPort);
    }
}

size_t I2SManager::write(const void *data, size_t size)
{
    if (!_isI2SRunning || _currentDevice == DEVICE_ADC_ONLY)
        return 0;
    size_t bytes_written = 0;
    i2s_write(_i2sPort, data, size, &bytes_written, 0);
    return bytes_written;
}
// size_t I2SManager::read(void *dest, size_t size)
// {
//     size_t bytesRead = 0;
//     static int8_t ADC_Read[64];

//     i2s_read(_i2sPort, &ADC_Read, 32, &bytesRead, portMAX_DELAY);
//     if (bytesRead > 0)
//     {
//         char logBuffer[512]; // Буфер на стеке или static
//         int offset = 0;
//         offset += sprintf(logBuffer, "[i2s] 32bit: ");

//         for (int i = 0; i < 32; i += 4) // Читаем по 4 байта (1 семпл)
//         {
//             uint32_t val = ((uint32_t)ADC_Read[i + 3] << 24) | ((uint32_t)ADC_Read[i + 2] << 16) | ((uint32_t)ADC_Read[i + 1] << 8) | ADC_Read[i];
//             int32_t pcm24 = (int32_t)val >> 8; // нормализация в 24-битное знаковое значение
//             offset += sprintf(logBuffer + offset, "[%08X / %d] ", val, pcm24);
//         }
//         PrintLogln(String(logBuffer));
//     }
//     return 0;
// }
size_t I2SManager::read(void *dest, size_t size)
{
    if (!_isI2SRunning || _currentDevice == DEVICE_DAC_ONLY)
        return 0;

    // Сколько 16-битных сэмплов от нас хотят (size байт / 2 байта на сэмпл)
    size_t samples_requested = size / sizeof(int16_t);
    // Сколько байт в 32-битных словах нам нужно выгрести из DMA
    size_t target_i2s_bytes = samples_requested * sizeof(int32_t);

    // Буфер под сырые 32-битные данные (выровнен по 4 байта)
    static int32_t i2s_accumulator[700];
    static size_t accumulated_bytes = 0;

    if (target_i2s_bytes > sizeof(i2s_accumulator))
    {
        return 0; // Защита от выхода за границы буфера
    }

    size_t bytes_to_read = target_i2s_bytes - accumulated_bytes;
    size_t chunk_read = 0;

    uint8_t *write_ptr = (uint8_t *)i2s_accumulator + accumulated_bytes;

    // Забираем сырой поток из DMA
    i2s_read(_i2sPort, write_ptr, bytes_to_read, &chunk_read, 0);
    accumulated_bytes += chunk_read;

    // if (chunk_read > 0)
    // {

    //     String outStr = "[i2s] Первые байты 32бит аудио: ";
    //     for (int i = 0; i < 128; i++)
    //     {
    //         outStr += String(write_ptr[i]) + " ";
    //     }
    //     PrintLogln(outStr);
    // }

    // Как только накопили полный пакет данных — обрабатываем
    if (accumulated_bytes >= target_i2s_bytes)
    {
        int16_t *dst = (int16_t *)dest;

        for (size_t i = 0; i < samples_requested; i++)
        {
            // Сдвигаем на 16 бит вправо.
            // Младшие мусорные байты (где жили 0x80 и 0x00) полностью стираются.
            // Остаются только истинные старшие 16 бит сэмпла АЦП.
            dst[i] = (int16_t)(i2s_accumulator[i] >> 16);
        }

        accumulated_bytes = 0; // Сброс для следующего цикла чтения
        return size;
    }

    return 0;
}
