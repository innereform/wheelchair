#define fwupdate.h

#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "cert.h"

String model = "wheel";
String pmode = "_production";
//String pmode ="_testing"; // https://raw.githubusercontent.com/innereform/clubfinder/master/proto_version_production.txt
#define URL_fw_Version "https://raw.githubusercontent.com/innereform/wheelchair/master/"+String(model)+String("_version")+String(pmode)+String(".txt") // 
#define URL_fw_Bin "https://raw.githubusercontent.com/innereform/wheelchair/master/"+String(model)+String(pmode)+String(".bin")
const char* host = "raw.githubusercontent.com";
String FirmwareVer = {
  "0.1"
};
const int httpsPort = 443;





extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

void setClock() {
   // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

}

int FirmwareVersionCheck(void) {
  String payload;
  int httpCode;
  String fwurl = URL_fw_Version;
  fwurl += "?";
  fwurl += String(rand());
  Serial.println(fwurl);
  WiFiClientSecure * client = new WiFiClientSecure;

  if (client) 
  {
    client -> setCACert(rootCACertificate);

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
    HTTPClient https;

    if (https.begin( * client, fwurl)) 
    { // HTTPS      
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      delay(100);
      httpCode = https.GET();
      delay(100);
      if (httpCode == HTTP_CODE_OK) // if version received
      {
        payload = https.getString(); // save received version
      } else {
        Serial.print("error in downloading version file:");
        Serial.println(httpCode);
        return 2;
      }
      https.end();
    }
    delete client;
  }
      
  if (httpCode == HTTP_CODE_OK) // if version received
  {
    payload.trim();
    if (payload.equals(FirmwareVer)) {
      Serial.printf("\nDevice already on latest firmware version:%s\n", FirmwareVer);
      return 0;
    } 
    else 
    {
      Serial.println(payload);
      Serial.println("New firmware detected");
      return 1;
    }
  } 
  return 0;  
}

void firmwareUpdate() {
  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
//  httpUpdate.setLedPin(LED_BUILTIN, LOW);
  t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);

  switch (ret) {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    Serial.println("HTTP_UPDATE_OK");
    break;
  }
}
//int FirmwareUpdate()
//{
//  WiFiClientSecure client;
//  client.setCACert(&cert);
//  if (!client.connect(host, httpsPort)) {
//    Serial.println("Connection failed");
//    return 0;
//  }
//  client.print(String("GET ") + URL_fw_Version + " HTTP/1.1\r\n" +
//               "Host: " + host + "\r\n" +
//               "User-Agent: BuildFailureDetectorESP8266\r\n" +
//               "Connection: close\r\n\r\n");
//  while (client.connected()) {
//    String line = client.readStringUntil('\n');
//    if (line == "\r") {
//      //Serial.println("Headers received");
//      break;
//    }
//  }
//  String payload = client.readStringUntil('\n');
//
//  payload.trim();
//  if(payload.equals(FirmwareVer) )
//  {   
//     Serial.println("Device already on latest firmware version"); 
//     return 1;
//  }
//  else
//  {
//    Serial.println("New firmware detected");
//    for(int i =0;i<=20;i++){
//      digitalWrite(pwled1, !(digitalRead(pwled1)));
//      digitalWrite(pwled2, !(digitalRead(pwled2)));
//      digitalWrite(pwled3, !(digitalRead(pwled3)));
//      delay(70);
//    }
////    digitalWrite(pwled, !(digitalRead(pwled)));
//    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW); 
//    t_httpUpdate_return ret = ESPhttpUpdate.update(client, URL_fw_Bin);
//        
//    switch (ret) {
//      case HTTP_UPDATE_FAILED:
//        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
//        break;
//
//      case HTTP_UPDATE_NO_UPDATES:
//        Serial.println("HTTP_UPDATE_NO_UPDATES");
//        break;
//
//      case HTTP_UPDATE_OK:
//        Serial.println("HTTP_UPDATE_OK");
//        break;
//    }
//    return 2; 
//  }
// }  
