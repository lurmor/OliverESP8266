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
  }
  if (client.connected() == false)
  {
    SetSMFlag(GlobalSMFlags, TCPCONECTED, false);
    PrintLog("Conection lost");
  }
}

void ProcessServerRec(String line)
{
}

void S_SyncTime()
{

  unsigned long t = getAveragedTime();
  if (t != 0)
  {
    int error = GetRealTime() - t;
    if (unixTimeShift == 0)
      error = 0;
    unixTimeShift = t - millis();
    PrintLog("Time error :");
    PrintLog(error);
    if (error > 100)
    {
      ForceNTPUpdate();
      PrintLogln("Time Speed error > 100 !!!!!!!!!!!!!!!!!!!!");
    }
    else
    {
      double timeSpeedError = (double)error / millis();
      PrintLog("Time Speed error :");
      PrintLog(timeSpeedError);
      timeSpeed = timeSpeed - timeSpeedError / 2;
    }

    // Serial.print
  }
  else
  {
  }
  // unsigned long circleTime = GetRealTime();
  // if (circleLastTime != 0)
  // {
  //   PrintLog("Time Circle = ");
  //   PrintLog(circleTime - circleLastTime);
  // }
  // circleLastTime = circleTime;

  // else
  /// PrintLogln("NTP sync failed");
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

  static Transition NTPtoTCP(&TCPUpdate, {AUTOTRSN, AUTOTRSN});
  NTPUpdate.AddTrsn(&NTPtoTCP);

  mainSM = new stateMashine(&Start, &Any);

  delay(1000);
}
void loop()
{
  mainSM->SMIteration();
}