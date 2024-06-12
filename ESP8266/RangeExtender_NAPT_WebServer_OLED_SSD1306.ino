#include <ESP8266TrueRandom.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WebServer.h>

#include <Ed25519.h>
#include <arduino_base64.hpp>

//#include <ESP8266HTTPClient.h>
//#include <WiFiClientSecureBearSSL.h>
//#include <WiFiClientSecure.h>

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
void handleRngNum() {
  int i;
  String message = "Rng:";
  for(i = 0; i < 1000; i++)
  {
      long long x = ESP8266TrueRandom.random(1000);
      message +=",";
      message += x;
  }
  http_server.send(200, "text/plain", message);

}

void handleKeyGen(){

  uint8_t privateKey[32];
  uint8_t publicKey[32];
  uint8_t i;

  Ed25519::generatePrivateKey(privateKey);
  Ed25519::derivePublicKey(publicKey, privateKey);

  String strPrivate;
  String strPublic;
  String message = "\nPrivateKey:\n";

  message += "\n-----BEGIN OPENSSH PRIVATE KEY-----\n";

  auto inputLength = sizeof(privateKey);
  // Rename the namespace from base64_encode module to nkbase64 since it is declared by the web server
  char output[nkbase64::encodeLength(inputLength);];
  nkbase64::encode(privateKey, inputLength, output);
  
  message += (String)output;
  message +="\n-----END OPENSSH PRIVATE KEY-----\n";
  message += "\nPublicKey:\n";

  message += "\nsh-ed25519 ";
  for (i = 0; i < (uint8_t)(sizeof(publicKey)); i++) {
    if (publicKey[i] < 0x10) { /* To print "0F", not "F" */
      message += "0";
    }
    char buff[32]; sprintf(buff, "%x", publicKey[i]);
    message += buff;
  }
  message += " esp-generated\n";
  http_server.send(200, "text/plain", message);
}

void wifiReconnect(){
  
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(50);
  }

  auto& server = WiFi.softAPDhcpServer();
  server.setDns(WiFi.dnsIP(0));
}

const char* github_host = "api.github.com";
const uint16_t github_port = 443;

void httpsSendData(){

}
  
void setup() {
  randomSeed(ESP8266TrueRandom.random());
  Serial.begin(115200, SERIAL_8N1);
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_squeezed_r6_tr);
  
  // Sync the system time via NTP.
  // configTime(9 * 60 * 60, 0, "ntp.jst.mfeed.ad.jp", "ntp.nict.jp", "time.google.com");

  // first, connect to STA so we can get a proper local DNS server
  WiFi.mode(WIFI_STA);
  
  wifiMulti.addAP("BarLogata-2.0", "61330069");   // add Wi-Fi networks you want to connect to
  wifiMulti.addAP("BarLogata", "");
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
  http_server.on("/rng/num", handleRngNum);
  http_server.on("/rng/keygen", handleKeyGen);
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
      delay(1000);
      httpsSendData();
    }

//    delay(1000);
}
