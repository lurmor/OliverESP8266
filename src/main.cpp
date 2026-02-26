// #include <Arduino.h>
// #include <ESP8266WiFi.h>
// // #include <ESP8266mDNS.h>
// #include "..\lib\ArduinoMDNS-master\MDNS.h"
// #include <WiFiUdp.h>

// #ifndef STASSID
// #define STASSID "gachi24"
// #define STAPSK "mityagay"
// #endif

// unsigned int localPort = 5353;  // local port to listen on
// unsigned int ptpPort = 8880;  // local port to listen on

// // buffers for receiving and sending data
// char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];  // buffer to hold incoming packet,
// char ReplyBuffer[] = "acknowledged\r\n";        // a string to send back

// WiFiUDP Udp;
// WiFiUDP UdpPTP;
//   MDNS mdns(Udp);
//   void serviceFound(const char* type, MDNSServiceProtocol proto,
//                   const char* name, IPAddress ip, unsigned short port,
//                   const char* txtContent);


// void setup() {
//   Serial.begin(115200);
//   Serial.println("started");
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(STASSID, STAPSK);
//   while (WiFi.status() != WL_CONNECTED) {
//     Serial.print('.');
//     delay(500);
//   }
//   Serial.print("Connected! IP address: ");
//   Serial.println(WiFi.localIP());
//   // Serial.printf("UDP server on port %d\n", localPort);
//   // Udp.begin(localPort);
//   // UdpPTP.begin(localPort);

//  mdns.begin(WiFi.localIP(), "arduino");
//   mdns.setServiceFoundCallback(serviceFound);
// }

// void loop() {
//   // if there's data available, read a packet
//   // Udp.beginMulticast(WiFi.localIP(), IPAddress(224, 0, 0, 251), 5353);


//   // int packetSize = Udp.parsePacket();
//   // if (packetSize) {
//   //   Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort(), Udp.destinationIP().toString().c_str(), Udp.localPort(), ESP.getFreeHeap());

//   //   // read the packet into packetBufffer
//   //   int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
//   //   packetBuffer[n] = 0;
//   //   Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    
//   //   Udp.write(ReplyBuffer);
//   //   Udp.endPacket();


//   //   Serial.println("Contents:");
//   //   Serial.println(packetBuffer);
//   // }

  

//   // int ptpSize = UdpPTP.parsePacket();
//   // if(ptpSize){
//   //   uint64 T1;
//   //   int n = UdpPTP.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
//   //   T1 = (uint64) packetBuffer;
//   //   packetBuffer[n] = 0;
//   //   UdpPTP.beginPacket(UdpPTP.remoteIP(), UdpPTP.remotePort());
//   //   UdpPTP.write(micros64());
//   //   Udp.endPacket();

//   // }

//   //Serial.println();



//   // int n = MDNS.queryService("http", "tcp",500);
//   // Serial.println(n);
//   // if (n == 0) {
//   //    Serial.println("not found");
//   // } else {
//   //   for (int i = 0; i < n; ++i) {
//   //     Serial.println(MDNS.hostname(i));
//   //     Serial.println(MDNS.IP(i));
//   //   }
//   // }

//     char serviceName[] = "_rtp\0";

  

//   if (!mdns.isDiscoveringService()) {
   
//       //Serial.print("Discovering services of type '");
//       //Serial.print(serviceName);
//       //Serial.println("' via Multi-Cast DNS (Bonjour)...");

//       mdns.startDiscoveringService(serviceName,
//                                    MDNSServiceTCP,
//                                    5000);
    
//   }
// mdns.run();
// delay(500);
// }


// void serviceFound(const char* type, MDNSServiceProtocol /*proto*/,
//                   const char* name, IPAddress ip,
//                   unsigned short port,
//                   const char* txtContent)
// {
//   if (NULL == name) {
// 	Serial.print("Finished discovering services of type ");
// 	Serial.println(type);
//   } else {
//     Serial.print("Found: '");
//     Serial.print(name);
//     Serial.print("' at ");
//     Serial.print(ip);
//     Serial.print(", port ");
//     Serial.print(port);
//     Serial.println(" (TCP)");
//     if (txtContent) {
//       Serial.print("\ttxt record: ");
      
//       char buf[256];
//       char len = *txtContent++;
//       int i=0;
//       while (len) {
//         i = 0;PTP
//         while (len--)
//           buf[i++] = *txtContent++;
//         buf[i] = '\0';
//         Serial.print(buf);
//         len = *txtContent++;
        
//         if (len)
//           Serial.print(", ");
//         else
//           Serial.println();
//       }
//     }
//   }
// }