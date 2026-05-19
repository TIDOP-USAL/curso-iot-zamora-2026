/**
 * Curso IoT 05/2026 Zamora. USAL
 *
 * Práctica 5. Cliente HTTP y webhook.site
 *
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

WiFiMulti wifiMulti;

// Variables a modificar
#define WIFI_SSID "CursoIoT_2026"
#define WIFI_PASS "CursoIoT_2026"
#define HTTP_URL "http://webhook.site/tu-propia-direccion"

uint8_t getMacAddress(char *mac){
  uint8_t baseMac[20];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    return sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X\n",
      baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  }
  Serial.println("Failed to read MAC address");
  return 0;
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  delay(1000);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.STA.begin();

  char mac[20];
  Serial.print("XIAO MAC ");
  getMacAddress((char *)&mac);
  Serial.printf("%s\n", mac);

  Serial.println("[Wi-Fi] Connecting to AP");
  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);
}

void loop() {
  // wait for WiFi connection
  if ((wifiMulti.run() != WL_CONNECTED)) {
    delay(1000);
    return;
  }
  Serial.println("[Wi-Fi] Connected!");

  HTTPClient http;

  // Configure target server and url
  http.begin(HTTP_URL);  //HTTP

  Serial.print("[HTTP] GET...\n");
  // Start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode <= 0) {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return;
  }

  // HTTP header has been send and Server response header has been handled
  Serial.printf("[HTTP] GET... code: %d\n", httpCode);

  // OK response from server
  if (httpCode == HTTP_CODE_OK) {
    Serial.printf("[HTTP] GET OK!\n");
    /*String payload = http.getString();
    Serial.println(payload);*/
  }

  http.end();
  Serial.println("Done!");

  while(1) {
    delay(10000);
  }
}
