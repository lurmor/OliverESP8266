#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "findmDNS.h"
#include "stateMashine.h"
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
unsigned int circlesPerSec = 0;

void ProcessServerRec(String line);

void T_PrepareConectWIFI()
{
  PrintLog("WIFI " STASSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
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

  if (client.available())
  {
    String line = client.readStringUntil('\r');
    PrintLog("Received: " + line);
    ProcessServerRec(line);
  }
  if (client.connected() == false)
  {
    SetSMFlag(GlobalSMFlags, TCPCONECTED, false);
    PrintLog("Conection lost");
  }
}

void ProcessServerRec(String line)
{
  if (line[2] == 'D')
  {
    if (line[3] == 'S')
    {
      SetSMFlag(GlobalSMFlags, TRANSIVEROPEN, false);
      SetSMFlag(GlobalSMFlags, RESSIVEROPEN, false);
    }
    if (line[3] == 'T')
    {

      line = line.substring(4);
      IPAddress ip = IPAddress();
      if (ip.fromString(line))
      {
        SetSMFlag(GlobalSMFlags, TRANSIVEROPEN, true);
        udpSend->SetRemoteIP(ip);
      }
    }
    if (line[3] == 'R')
    {
      SetSMFlag(GlobalSMFlags, RESSIVEROPEN, true);
    }
  }
}

void S_SyncTime()
{

  TrySincTime();

  unsigned long circleTime = GetRealTime();

  // if (circleLastTime != 0)
  // {
  if (circleTime - circleLastTime > 1000)
  {

    PrintLog("CPS = ");
    PrintLogln(circlesPerSec);
    circlesPerSec = 0;
    circleLastTime = circleTime;
  }
  else
  {
    circlesPerSec++;
  }
  // }

  // else
  /// PrintLogln("NTP sync failed");
}

void S_UDPSpamTest()
{
  byte buf[1000];
  for (int i = 0; i < 1000; i++)
    buf[i] = 100;
  if (!udpSend->Send(buf, 1000))
  {
    PrintLogln("SendERROR");
  }
  else
  {
    // PrintLogln("DataSended");
  }
}

void setup()
{
  Serial.begin(115200);
#ifdef WITH_GDB
  gdbstub_init();
#endif
  delay(1000);

  PrintLogln("started");

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

  static Transition ConWIFItoMdns(&mDNS, WIFICONECTED);
  WIFI.AddTrsn(&ConWIFItoMdns);

  static Transition SatrtToWIFI(&WIFI, T_PrepareConectWIFI);
  Start.AddTrsn(&SatrtToWIFI);

  static Transition MdnsToTCP(&TCPParing, MDNSFINDED);
  mDNS.AddTrsn(&MdnsToTCP);

  static Transition TCPconected(&TCPUpdate, TCPCONECTED);
  TCPParing.AddTrsn(&TCPconected);
  static Transition TCPdisconected(&TCPParing, {AUTOTRSN, TCPCONECTED});
  TCPUpdate.AddTrsn(&TCPdisconected);

  static Transition TCPtoNTP(&NTPUpdate, {AUTOTRSN, AUTOTRSN});
  TCPUpdate.AddTrsn(&TCPtoNTP);

  static Transition NTPtoIN(&Input, RESSIVEROPEN);
  NTPUpdate.AddTrsn(&NTPtoIN);

  static Transition NTPtoOUT(&Output, TRANSIVEROPEN);
  NTPUpdate.AddTrsn(&NTPtoOUT);

  static Transition NTPtoTCP(&TCPUpdate, {AUTOTRSN, AUTOTRSN});
  NTPUpdate.AddTrsn(&NTPtoTCP);
  static Transition INtoTCP(&TCPUpdate, {AUTOTRSN, AUTOTRSN});
  Input.AddTrsn(&INtoTCP);
  static Transition OuttoTCP(&TCPUpdate, {AUTOTRSN, AUTOTRSN});
  Output.AddTrsn(&OuttoTCP);

  mainSM = new stateMashine(&Start, &Any);
  udpRes = new UdpReceiver(UDP_AUDIO_IN_PORT);
  // udpRes->Begin();
  udpSend = new UdpSender(UDP_AUDIO_IN_PORT, UDP_AUDIO_OUT_PORT);
  // udpSend->Begin();

  delay(1000);
}
void loop()
{
  mainSM->SMIteration();
}