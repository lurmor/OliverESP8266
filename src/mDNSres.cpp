// #include <Arduino.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h>
    
// const char* ssid = "gachi24";
// const char* password =  "mityagay";
       
// void setup(){
//   Serial.begin(115200);
    
//   WiFi.begin(ssid, password);
    
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.println("Connecting to WiFi..");
//   }
   
//   if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
//     Serial.println("mDNS responder started");
//   } else {
//     Serial.println("Error setting up MDNS responder!");}
  
//   MDNS.addService("rtp", "tcp", 80);
 
//   Serial.println(WiFi.localIP());
    
// }
    
// void loop(){MDNS.update();}