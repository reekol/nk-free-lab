#include <ESP8266TrueRandom.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WebServer.h>

#include <Hash.h>
#include <WiFiUdp.h>
#include <TOTP.h>

#include <Ed25519.h>
#include <arduino_base64.hpp>

#define FLASH_BUTTON_PIN 0
//#include <ESP8266HTTPClient.h>
//#include <WiFiClientSecureBearSSL.h>
//#include <WiFiClientSecure.h>


   
// OTP secret keys

// convert python.py bellow
// import base64
// tokenFreeOtpExport  = [94, -61, 23, 59, 100, 29, 26, -84, -12, -119]
// secret = bytes((x + 256) & 255 for x in tokenFreeOtpExport); print(secret)
// b = list(secret); print(b)
// code = base64.b32encode(secret); print(code.decode())

byte secret1[] = { 194, 195, 123, 159, 100, 129, 126, 172, 244, 137 };
// byte secret2[] = { 0xae, 0x22, 0x2c, 0x6c, 0x9e, 0xf2, 0xcb, 0x5c, 0x02, 0x5c };

TOTP totp1 = TOTP(secret1, sizeof(secret1));
//TOTP totp2 = TOTP(secret2, sizeof(secret2));

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

void handleOtp() {
  int i;
  String message = "OTP:";
  String otpString1 = totp1.getCode(time (nullptr));
//  String otpString2 = totp2.getCode(time (nullptr));
  message += "\n" + otpString1;
//  message += "\n" + otpString2;
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
  char output[nkbase64::encodeLength(inputLength)];
  nkbase64::encode(privateKey, inputLength, output);
  
  message += (String)output;
  message +="\n-----END OPENSSH PRIVATE KEY-----\n";
  message += "\nPublicKey:\n";

  message += "\nssh-ed25519 ";

  message += "\n";
  
  auto inputLength2 = sizeof(publicKey);
  // Rename the namespace from base64_encode module to nkbase64 since it is declared by the web server
  char output2[nkbase64::encodeLength(inputLength2)];
  nkbase64::encode(publicKey, inputLength2, output2);
  message += (String)output2;
  message += " esp-generated\n";
  message += "\n\n" + sha1("abc");  
  
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

void showInfoScreen(){
    u8g2.setFont(u8g2_font_squeezed_r6_tr);
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
}

void setup() {
  pinMode(FLASH_BUTTON_PIN, INPUT_PULLUP);
  randomSeed(ESP8266TrueRandom.random());
  Serial.begin(115200, SERIAL_8N1);
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_squeezed_r6_tr);
  
  // Sync the system time via NTP.
  configTime(1 * 60 * 60, 0, "ntp.jst.mfeed.ad.jp", "ntp.nict.jp", "time.google.com");

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
  http_server.on("/otp", handleOtp);
  http_server.onNotFound(handleNotFound);
  http_server.begin();
}


void loop() {
    http_server.handleClient();
    if (digitalRead(FLASH_BUTTON_PIN) == LOW){
      u8g2.clearBuffer();
      u8g2.drawStr(5,10, "OTP");
      u8g2.setFont(u8g2_font_lubB24_tn);
      u8g2.drawStr(0,60, totp1.getCode(time (nullptr)));
      u8g2.sendBuffer();
      u8g2.setFont(u8g2_font_squeezed_r6_tr);
    }
    
    if(WiFi.status() != WL_CONNECTED){
      u8g2.clearBuffer();
      u8g2.drawStr(5,10, "RECONNECTING");
      u8g2.sendBuffer();

      wifiReconnect();

      showInfoScreen();
      
      delay(1000);
      httpsSendData();
    }

//    delay(1000);
}
