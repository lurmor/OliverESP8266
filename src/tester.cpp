#include <Arduino.h>
// To32#include <ESP8266WiFi.h>
#if defined(ESP32)
#include <WiFi.h>
#include <esp_wifi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error "Неизвестная платформа! Выберите ESP8266 или ESP32."
#endif

#include <WiFiUdp.h>
#include "findmDNS.h"
#include "stateMashine.h"
#include "CircularArray.h"
#include "NTP.h"
#include "SenrResUDP.h"
/// #include "globalData.h"
#include "Debuging.h"

#ifdef WITH_GDB
#include <GDBStub.h>
#endif

#ifndef STASSID
#define STASSID "gachi24"
#define STAPSK "mityagay"
#endif

stateMashine *mainSM = nullptr;
UdpReceiver *udpRes = nullptr;
UdpSender *udpSend = nullptr;
unsigned long circleLastTime = 0;
unsigned int statesPerSec = 0;
unsigned int packetsPerSec = 0;
unsigned int errorsPerSec = 0;

QueueHandle_t audioInQueue;
QueueHandle_t audioOutQueue;
QueueHandle_t tcpIncomingQueue = NULL;

struct AudioBuffer
{
  uint8_t data[1400];
};

struct NetworkMessage
{
  char payload[256];
};
CircularArray<AudioBuffer> circArr(16);

void ProcessServerRec(String line);
void AudioCaptureTask(void *pvParameters);
void NetworkTask(void *pvParameters);
void RecoverMissingPacketAdvanced(const AudioBuffer &prev, const AudioBuffer &next, AudioBuffer &missing);

void T_PrepareConectWIFI()
{
  PrintLog("WIFI " STASSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
}
void T_PostConectWIFI()
{
  WiFi.setSleep(false);

  // Проверяем, применилась ли настройка в железе
  esp_err_t err = esp_wifi_set_ps(WIFI_PS_NONE);
  if (err == ESP_OK)
  {
    Serial.println("SUCCESS: Wi-Fi Power Save is DISABLED!");
  }
  else
  {
    Serial.printf("ERROR: Failed to disable Power Save, code: %d\n", err);
  }

  udpRes = new UdpReceiver(UDP_AUDIO_IN_PORT);
  udpRes->Begin();
  udpSend = new UdpSender(UDP_AUDIO_IN_PORT, UDP_AUDIO_OUT_PORT);
}

void S_IDLE()
{
  static int packetCounter = 0;
  packetCounter++;
  if (packetCounter >= 1000)
  {
    packetCounter = 0;
    vTaskDelay(1); // Сбросит Watchdog и даст системе подышать
  }

  unsigned long circleTime = GetRealTime();

  // if (circleLastTime != 0)
  // {
#ifdef STAT
  if (circleTime - circleLastTime > 1000)
  {

    PrintLog("SPS = ");
    PrintLogln(statesPerSec);
    PrintLog("PPS = ");
    PrintLogln(packetsPerSec);

    if (errorsPerSec != 0)
    {
      PrintLog("EPS = ");
      PrintLogln(errorsPerSec);
    }

    statesPerSec = 0;
    packetsPerSec = 0;
    errorsPerSec = 0;
    circleLastTime = circleTime;
  }
#endif
}

void S_ConectWIFI()
{
  PrintLog(WiFi.status());
  if (WiFi.status() == WL_CONNECTED)
  {
    SetSMFlag(GlobalSMFlags, WIFICONECTED, true);
    PrintLog("Connected! IP address: ");
    PrintLog(WiFi.localIP());
  }
  // PrintLog('.');
  delay(1000);
}

void S_FindmDNS()
{
  if (findmDNS(WiFi.localIP(), "Olivier._rtp._tcp.local", serverIP, tcpServerPort) >= 0)
  {
    SetSMFlag(GlobalSMFlags, MDNSFINDED, true);
    PrintLogln(serverIP.toString() + " " + tcpServerPort);
  }
}

void S_ConectTCP()
{
  if (client.connect(serverIP, tcpServerPort))
  {
    PrintLog("Connected to server!");
    client.println("R-" + String(SN) + "-" + String(UNITTYPE));
    SetSMFlag(GlobalSMFlags, TCPCONECTED, true);
  }
  else
  {
    PrintLogln("Connection to server failed.");
    delay(2000);
  }
}

void S_TCPUpdade()
{

  if (tcpIncomingQueue == NULL)
    return;

  NetworkMessage receivedMsg;

  // Проверяем очередь. Если там есть сообщение, забираем его (таймаут 0 — проверка без блокировки)
  while (xQueueReceive(tcpIncomingQueue, &receivedMsg, 0) == pdPASS)
  {
    // Вызываем ваш оригинальный обработчик строки
    ProcessServerRec(receivedMsg.payload);
  }
}

void ProcessServerRec(String line)
{
  PrintLogln("Server Comand " + line);
  if (line[0] == 'D')
  {
    if (line[1] == 'S')
    {
      SetSMFlag(GlobalSMFlags, TRANSIVEROPEN, false);
      SetSMFlag(GlobalSMFlags, RESSIVEROPEN, false);
    }
    if (line[1] == 'T')
    {

      line = line.substring(2);
      IPAddress ip = IPAddress();
      if (ip.fromString(line))
      {
        SetSMFlag(GlobalSMFlags, TRANSIVEROPEN, true);
        udpSend->SetRemoteIP(ip);
      }
    }
    if (line[1] == 'R')
    {
      SetSMFlag(GlobalSMFlags, RESSIVEROPEN, true);
    }
  }
}

void S_SyncTime()
{
  TrySincTime();

  // }

  // else
  /// PrintLogln("NTP sync failed");
}

void S_UDPSpamTest()
{
  const uint32_t packMicrosDelay = 1000000 / 128;
  static uint32_t lastPacket = micros();

  if (micros() - lastPacket >= packMicrosDelay)
  {
    lastPacket = micros();

    AudioBuffer currentBuffer;

    uint16_t count = 1392;
    static uint16_t ind = 0;
    uint16_t time = GetRealTimeMarker();

    currentBuffer.data[0] = (uint8_t)(count & 0xFF);
    currentBuffer.data[1] = (uint8_t)((count >> 8) & 0xFF);

    currentBuffer.data[2] = (uint8_t)(ind & 0xFF);
    currentBuffer.data[3] = (uint8_t)((ind >> 8) & 0xFF);

    currentBuffer.data[4] = (uint8_t)(time & 0xFF);
    currentBuffer.data[5] = (uint8_t)((time >> 8) & 0xFF);

    if (xQueueSend(audioOutQueue, &currentBuffer, 0) == pdPASS)
      ind++;
  }
}
void S_UDPParse()
{
  AudioBuffer abuffer;
  static SlideWindow<int> latensy(1);

  if (xQueueReceive(audioInQueue, &abuffer, 0) == pdPASS)
  {
    uint16_t count = abuffer.data[0] | (abuffer.data[1] << 8);
    uint16_t ind = abuffer.data[2] | (abuffer.data[3] << 8);
    uint16_t time = abuffer.data[4] | (abuffer.data[5] << 8);
    memcpy(circArr.get(ind), &abuffer, 1400);
    circArr.SetLast(ind);
    latensy.push(GetRealTimeMarker() - time);
    // PrintLog(latensy.getAverage());
  }
}

void S_PushAudio()
{
  static int pointer = 0;
  AudioBuffer zeroAudio;
  static AudioBuffer *currentAudio;
  static AudioBuffer *lastAudio;
  memset(&zeroAudio, 0, sizeof(AudioBuffer));

  uint16_t shiftMs = 30;

  currentAudio = circArr.get(pointer);

  Serial.printf(" %03u     ", currentAudio->data[2]);
  if (currentAudio->data[0] == 0)
  {
    PrintLogln("AUDIO LOST");
    AudioBuffer *nextAudio = circArr.get(pointer + 1);

    // ЗАЩИТА 2: Проверяем, что и прошлый, и следующий пакеты физически существуют в памяти
    if (lastAudio != nullptr && nextAudio != nullptr)
    {
      if (lastAudio->data[0] != 0 && nextAudio->data[0] != 0)
      {
        RecoverMissingPacketAdvanced(*lastAudio, *nextAudio, *currentAudio);
        PrintLogln("AUDIO Recover");
      }
      else
      {
        memcpy(lastAudio, &zeroAudio, 1400);
        lastAudio = currentAudio;
        pointer++;
      }
    }
    else
    {
      if (lastAudio != nullptr)
        memcpy(lastAudio, &zeroAudio, 1400);
      lastAudio = currentAudio;
      pointer++;
    }
  }
  else
  {
    uint16_t time = currentAudio->data[4] | (currentAudio->data[5] << 8);
    if (time + shiftMs - GetRealTimeMarker() <= 0)
    {

      for (size_t i = 0; i < circArr.size(); ++i)
      {
        auto ptr = circArr.get(i);
        Serial.printf(" %03u", ptr->data[2]);
      }

      pointer++;
      if (lastAudio != nullptr)
        memcpy(lastAudio, &zeroAudio, 1400);
      lastAudio = currentAudio;
    }
  }
  PrintLogln("");

  // PrintLogln("OVERLAP");
}

void setup()
{

  SN = (uint32_t)ESP.getEfuseMac();
  Serial.begin(115200);
#ifdef WITH_GDB
  gdbstub_init();
#endif
  delay(1000);
  PrintLogln("started");
  esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

  static State Start("Start");
  static State Any("any");
  static State WIFI("WIFI");
  static State TCPParing("TCPParing");
  static State TCPUpdate("TCPUpdate");
  static State NTPUpdate("NTPUpdate");
  static State mDNS("mDNS");
  static State IDLE("IDLE");
  static State Output("Out");
  static State Input("In");

  WIFI.addAction(S_ConectWIFI);
  mDNS.addAction(S_FindmDNS);
  TCPParing.addAction(S_ConectTCP);
  TCPUpdate.addAction(S_TCPUpdade);
  NTPUpdate.addAction(S_SyncTime);
  Output.addAction(S_UDPSpamTest);
  Input.addAction(S_UDPParse);
  Input.addAction(S_PushAudio);
  IDLE.addAction(S_IDLE);

  static Transition ConWIFItoMdns(&mDNS, T_PostConectWIFI, WIFICONECTED);
  WIFI.AddTrsn(&ConWIFItoMdns);

  static Transition SatrtToWIFI(&WIFI, T_PrepareConectWIFI);
  Start.AddTrsn(&SatrtToWIFI);

  static Transition MdnsToTCP(&TCPParing, MDNSFINDED);
  mDNS.AddTrsn(&MdnsToTCP);

  static Transition TCPconected(&TCPUpdate, TCPCONECTED);
  TCPParing.AddTrsn(&TCPconected);
  static Transition TCPdisconected(&TCPParing, {AUTOTRSN, TCPCONECTED});
  TCPUpdate.AddTrsn(&TCPdisconected);

  static Transition TCPtoIDLE(&IDLE, {AUTOTRSN, AUTOTRSN});
  TCPUpdate.AddTrsn(&TCPtoIDLE);

  static Transition IDLEtoNTP(&NTPUpdate, {AUTOTRSN, AUTOTRSN});
  IDLE.AddTrsn(&IDLEtoNTP);

  static Transition NTPtoIN(&Input, RESSIVEROPEN);
  NTPUpdate.AddTrsn(&NTPtoIN);

  static Transition INtoOUT(&Output, TRANSIVEROPEN);
  Input.AddTrsn(&INtoOUT);

  static Transition NTPtoOUT(&Output, TRANSIVEROPEN);
  NTPUpdate.AddTrsn(&NTPtoOUT);

  static Transition NTPtoTCP(&TCPUpdate, {AUTOTRSN, AUTOTRSN});
  NTPUpdate.AddTrsn(&NTPtoTCP);
  static Transition INtoTCP(&TCPUpdate, {AUTOTRSN, AUTOTRSN});
  Input.AddTrsn(&INtoTCP);
  static Transition OuttoTCP(&TCPUpdate, {AUTOTRSN, AUTOTRSN});
  Output.AddTrsn(&OuttoTCP);

  mainSM = new stateMashine(&Start, &Any);

  audioInQueue = xQueueCreate(10, sizeof(AudioBuffer));
  audioOutQueue = xQueueCreate(10, sizeof(AudioBuffer));
  tcpIncomingQueue = xQueueCreate(10, sizeof(NetworkMessage));

  // Подключаем Wi-Fi ...

  // Задача для ЯДРА 0: Сбор звука
  xTaskCreatePinnedToCore(
      AudioCaptureTask, // Функция задачи
      "AudioTask",      // Название
      16384,            // Размер стека
      NULL,             // Параметры
      3,                // Приоритет (высокий)
      NULL,             // Хэндл задачи
      0                 // ЖЕСТКО НА ЯДРО 0
  );

  // Задача для ЯДРА 1: Сеть и отправка
  xTaskCreatePinnedToCore(
      NetworkTask,
      "NetTask",
      4096,
      NULL,
      2, // Приоритет чуть ниже
      NULL,
      1 // ЖЕСТКО НА ЯДРО 1
  );

  delay(1000);
}

void CheckNetworkMemory()
{
  // Показывает общую свободную память, доступную для сетевых буферов
  uint32_t freeNetworkRam = heap_caps_get_free_size(MALLOC_CAP_8BIT);

  // Показывает самый большой цельный кусок памяти (если он маленький — память фрагментирована)
  uint32_t maxBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

  Serial.printf("[LwIP Memory] Free: %d bytes | Max Block: %d bytes\n", freeNetworkRam, maxBlock);
}

void AudioCaptureTask(void *pvParameters)
{

  // Инициализация I2S / АЦП здесь...

  for (;;)
  {
    // 1. Наполняем currentBuffer.data данными (4 канала, 44кГц)
    // В идеале здесь используется i2s_read(), который работает через DMA
    mainSM->SMIteration();
    statesPerSec++;

    // 2. Отправляем готовый буфер в очередь. Если очередь полна, ждем 1 тира.
  }
}
void NetworkTask(void *pvParameters)
{
  AudioBuffer bufferToSend;
  byte header[2];
  byte buf[256];
  uint32_t lastConnCheck = 0;
  int targetPPS = 512;
  const TickType_t xFrequency = pdMS_TO_TICKS(1000.0 / targetPPS);
  TickType_t xLastWakeTime;

  xLastWakeTime = xTaskGetTickCount();

  bool hasPendingAudio = false; // Хранит статус: есть ли у нас застрявший пакет

  for (;;)
  {

    if (millis() - lastConnCheck > 100)
    {
      lastConnCheck = millis();
      if (!client.connected())
      {
        SetSMFlag(GlobalSMFlags, TCPCONECTED, false);
      }
    }
    int availableBytes = client.available();
    if (availableBytes >= 2)
    {
      client.read(header, 2);
      uint8_t packetLen = header[1];

      if (packetLen >= 2 && packetLen <= 254)
      {
        // Микро-ожидание тела пакета, чтобы не порвать буфер
        uint32_t timeout = millis();
        while (client.available() < packetLen && (millis() - timeout < 10))
        {
          vTaskDelay(pdMS_TO_TICKS(1));
        }

        if (client.available() >= packetLen)
        {
          client.read(buf, packetLen);
          buf[packetLen] = '\0';

          NetworkMessage msg;
          memcpy(msg.payload, buf, packetLen + 1);
          xQueueSend(tcpIncomingQueue, &msg, 0);
        }
      }
    }

    if (udpRes != NULL)
    {
      int len = udpRes->Update();
      if (len > 0)
      {
        auto msg = udpRes->GetData();

        // Добавляем static. Переменная больше НЕ живет на стеке!
        static AudioBuffer abuffer;

        // Очищаем старые данные перед копированием (на всякий случай)
        memset(&abuffer, 0, sizeof(AudioBuffer));

        memcpy(abuffer.data, msg, len);

        xQueueSend(audioInQueue, &abuffer, 0);
        packetsPerSec++;
      }
    }

    if (udpSend != NULL)
    {

      // Шаг А: Берем пакет из очереди (без блокировки, t=0)
      if (!hasPendingAudio)
      {
        if (xQueueReceive(audioOutQueue, &bufferToSend, 0) == pdPASS)
        {
          hasPendingAudio = true;
        }
      }

      // Шаг Б: Отправляем
      if (hasPendingAudio)
      {
        if (udpSend->Send(bufferToSend.data, 1400))
        {
          packetsPerSec++;
          hasPendingAudio = false; // Успех
        }
        else
        {
          errorsPerSec++; // Поймали ошибку 12

          // Сеть занята. Чтобы не повесить Ядро 0, даем ему
          // дополнительный микро-отдых прямо здесь
          vTaskDelay(1);
          xLastWakeTime = xTaskGetTickCount();
        }
      }
    }
    xTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

void loop()
{
  // Пустой! Ардуиновский loop работает на Ядре 1, но мы ушли во FreeRTOS задачи.
  vTaskDelete(NULL);
}

void RecoverMissingPacketAdvanced(const AudioBuffer &prev, const AudioBuffer &next, AudioBuffer &missing)
{
  const size_t audioOffset = 6;
  const size_t numSamples = (1400 - audioOffset) / sizeof(int16_t) / 2; // 348 стерео-семплов

  const int16_t *prevSamples = reinterpret_cast<const int16_t *>(prev.data + audioOffset);
  const int16_t *nextSamples = reinterpret_cast<const int16_t *>(next.data + audioOffset);
  int16_t *missingSamples = reinterpret_cast<int16_t *>(missing.data + audioOffset);

  const uint16_t *prevHeader = reinterpret_cast<const uint16_t *>(prev.data);
  const uint16_t *nextHeader = reinterpret_cast<const uint16_t *>(next.data);
  uint16_t *missingHeader = reinterpret_cast<uint16_t *>(missing.data);
  missingHeader[0] = prevHeader[0];
  missingHeader[1] = static_cast<uint16_t>(prevHeader[1] + 1);
  missingHeader[2] = static_cast<uint16_t>(prevHeader[2] / 2 + nextHeader[2] / 2);

  for (size_t i = 0; i < numSamples; ++i)
  {
    float alpha = static_cast<float>(i) / static_cast<float>(numSamples - 1);

    float weightPrev = 1.0f - alpha;
    float weightNext = alpha;

    int16_t srcPrevL = prevSamples[i * 2];
    int16_t srcPrevR = prevSamples[i * 2 + 1];

    int16_t srcNextL = nextSamples[i * 2];
    int16_t srcNextR = nextSamples[i * 2 + 1];

    float mixedL = (srcPrevL * weightPrev) + (srcNextL * weightNext);
    missingSamples[i * 2] = static_cast<int16_t>(mixedL);

    float mixedR = (srcPrevR * weightPrev) + (srcNextR * weightNext);
    missingSamples[i * 2 + 1] = static_cast<int16_t>(mixedR);
  }
}
