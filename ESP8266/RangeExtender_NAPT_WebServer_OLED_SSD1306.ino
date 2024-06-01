
// NAPT example released to public domain

#if LWIP_FEATURES && !LWIP_IPV6

#define HAVE_NETDUMP 0

#ifndef STASSID
#define STASSID "BarLogata-2.0"
#define STAPSK "61330069"
#define STAPSKEXT "76543210"
#define EXTNAME "_GUEST"
#endif

#include <ESP8266WiFi.h>
#include <lwip/napt.h>
#include <lwip/dns.h>


#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,/*clock=*/14,/*data*/12, U8X8_PIN_NONE);

#include <ESP8266WebServer.h>
ESP8266WebServer http_server(80);

#define NAPT 1000
#define NAPT_PORT 10

#if HAVE_NETDUMP

#include <NetDump.h>


void dump(int netif_idx, const char* data, size_t len, int out, int success) {
  (void)success;
  Serial.print(out ? F("out ") : F(" in "));
  Serial.printf("%d ", netif_idx);

  // optional filter example: if (netDump_is_ARP(data))
  {
    netDump(Serial, data, len);
    // netDumpHex(Serial, data, len);
  }
}
#endif

void handleRoot() {
  http_server.send(200, "text/html", "<h1>You are connected</h1>");
}
void setup() {
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_tiny5_tf);

  Serial.begin(115200);
  Serial.printf("\n\nNAPT Range extender\n");
  Serial.printf("Heap on start: %d\n", ESP.getFreeHeap());

  u8g2.drawStr(6,10,"Extender:");
  u8g2.sendBuffer();

#if HAVE_NETDUMP
  phy_capture = dump;
#endif

  // first, connect to STA so we can get a proper local DNS server
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  u8g2.drawStr(42,10, WiFi.localIP().toString().c_str());
  u8g2.sendBuffer();

  Serial.printf("\nSTA: %s (dns: %s / %s)\n", WiFi.localIP().toString().c_str(), WiFi.dnsIP(0).toString().c_str(), WiFi.dnsIP(1).toString().c_str());

  http_server.on("/", handleRoot);
  http_server.begin();
  u8g2.drawStr(0,10, ": ");
  u8g2.sendBuffer();
 
  // By default, DNS option will point to the interface IP
  // Instead, point it to the real DNS server.
  // Notice that:
  // - DhcpServer class only supports IPv4
  // - Only a single IP can be set
  auto& server = WiFi.softAPDhcpServer();
  server.setDns(WiFi.dnsIP(0));

  WiFi.softAPConfig(  // enable AP, with android-compatible google domain
    IPAddress(172, 217, 28, 254), IPAddress(172, 217, 28, 254), IPAddress(255, 255, 255, 0));
  WiFi.softAP(STASSID EXTNAME, STAPSKEXT);
  Serial.printf("AP: %s\n", WiFi.softAPIP().toString().c_str());

  Serial.printf("Heap before: %d\n", ESP.getFreeHeap());
  err_t ret = ip_napt_init(NAPT, NAPT_PORT);
  Serial.printf("ip_napt_init(%d,%d): ret=%d (OK=%d)\n", NAPT, NAPT_PORT, (int)ret, (int)ERR_OK);
  if (ret == ERR_OK) {
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);
    if (ret == ERR_OK) { Serial.printf("WiFi Network '%s' with same password is now NATed behind '%s'\n", STASSID "extender", STASSID); }
  }
  Serial.printf("Heap after napt init: %d\n", ESP.getFreeHeap());
  if (ret != ERR_OK) { Serial.printf("NAPT initialization failed\n"); }
  u8g2.drawStr(0,10, "::");
  u8g2.sendBuffer();

}

#else

void setup() {
  WebSerial.println("Hello!");
  Serial.begin(115200);
  Serial.printf("\n\nNAPT not supported in this configuration\n");
}

#endif

void loop() {
    http_server.handleClient();
}
