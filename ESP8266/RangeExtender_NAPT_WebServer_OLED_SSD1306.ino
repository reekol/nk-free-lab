#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
//#include <WiFiClientSecureBearSSL.h>
#include <WiFiClientSecure.h>

#define STAPSKEXT "76543210"
#define EXTNAME "ESP_GUEST"
#define EMPTY_LINE "                    "
#define NAPT 1000
#define NAPT_PORT 10

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer http_server(80);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,/*clock=*/14,/*data*/12, U8X8_PIN_NONE);

void handleRoot() {
  http_server.send(200, "text/html", "<h1>You are connected</h1>");
}
void handleNotFound() {
  http_server.send(404, "text/html", "<h1>404, Monkey not found</h1>");
}

void wifiReconnect(){
  
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(50);
  }

  auto& server = WiFi.softAPDhcpServer();
  server.setDns(WiFi.dnsIP(0));
}

void httpsSendData(){

  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    
    client->setInsecure();
    //create an HTTPClient instance
    HTTPClient https;

    //Initializing an HTTPS communication using the secure client
    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://www.howsmyssl.com/a/check")) {  // HTTPS
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
       Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          // print server response payload
          String payload = https.getString();
          Serial.println(payload);
        }
      }
      else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    }
  }
  else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
}
  
void setup() {
  Serial.begin(115200, SERIAL_8N1);
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_squeezed_r6_tr);
  
  // Sync the system time via NTP.
  // configTime(9 * 60 * 60, 0, "ntp.jst.mfeed.ad.jp", "ntp.nict.jp", "time.google.com");

  // first, connect to STA so we can get a proper local DNS server
  WiFi.mode(WIFI_STA);
  
  wifiMulti.addAP("BarLogata-2.0", "61330069");   // add Wi-Fi networks you want to connect to
  wifiMulti.addAP("BarLogataGuest", "76543210");
  wifiMulti.addAP("moto g(8) plus 7366", "76543210");

  WiFi.softAPConfig(  // enable AP, with android-compatible google domain
    IPAddress(172, 217, 28, 254),
    IPAddress(172, 217, 28, 254),
    IPAddress(255, 255, 255, 0)
  );
  
  WiFi.softAP(EXTNAME, STAPSKEXT);
  Serial.print ("\n");
  Serial.printf("AP: %s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("Heap before: %d\n", ESP.getFreeHeap());
  err_t ret = ip_napt_init(NAPT, NAPT_PORT);
  Serial.printf("ip_napt_init(%d,%d): ret=%d (OK=%d)\n", NAPT, NAPT_PORT, (int)ret, (int)ERR_OK);
  if (ret == ERR_OK) {
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);
    if (ret == ERR_OK) { 
//    Serial.printf("WiFi Network '%s' with same password is now NATed behind '%s'\n");
    }
  }
  Serial.printf("Heap after napt init: %d\n", ESP.getFreeHeap());
  if (ret != ERR_OK) { Serial.printf("NAPT initialization failed\n"); }

  http_server.on("/", handleRoot);
  http_server.onNotFound(handleNotFound);
  http_server.begin();
}


void loop() {
    http_server.handleClient();
    if(WiFi.status() != WL_CONNECTED){
      u8g2.clearBuffer();
      u8g2.drawStr(5,10, "RECONNECTING");
      u8g2.sendBuffer();

      wifiReconnect();

      u8g2.clearBuffer();
      u8g2.drawStr(5, 10, EMPTY_LINE);
      u8g2.drawStr(5, 10, WiFi.localIP().toString().c_str());
      u8g2.drawStr(63,10, WiFi.SSID().c_str());

      u8g2.drawStr(5,25, EMPTY_LINE);
      u8g2.drawStr(5,25, WiFi.dnsIP(0).toString().c_str());
      u8g2.drawStr(5,35, EMPTY_LINE);
      u8g2.drawStr(5,35, WiFi.dnsIP(1).toString().c_str());
      u8g2.drawStr(5,45, EMPTY_LINE);
      u8g2.drawStr(5,45, WiFi.softAPIP().toString().c_str());
      u8g2.sendBuffer();
      delay(3000);
      httpsSendData();
    }

    delay(1000);
}
