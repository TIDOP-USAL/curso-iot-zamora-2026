/**
 * Curso IoT 05/2026 Zamora. USAL
 *
 * Práctica 6. Cliente HTTPS y ThingSpeak. Envío de datos en formato JSON
 * Instala: Arduino_JSON 0.2.0
 *
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

// Variables a modificar
#define WIFI_SSID "CursoIoT_2026"
#define WIFI_PASS "CursoIoT_2026"
#define HTTPS_URL "https://api.thingspeak.com/update"
#define THINGSPEAK_API_KEY ""

uint8_t getMacAddress(char *mac){
  uint8_t baseMac[20];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    return sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
      baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  }
  Serial.println("Failed to read MAC address");
  return 0;
}

// DigiCert Global Root G2
const char *rootCACertificate = R"string_literal(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----
)string_literal";

void setClock() {
  configTime(0, 0, "pool.ntp.org");

  Serial.print(F("[NTP] Waiting for time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("[NTP] Current time: "));
  char buf[26];
  Serial.print(asctime_r(&timeinfo, buf));
}

WiFiMulti wifiMulti;
char mac[20];

void setup() {
  Serial.begin(115200);
  while(!Serial);
  delay(1000);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.STA.begin();

  Serial.print("XIAO MAC ");
  getMacAddress((char *)&mac);
  Serial.printf("%s\n", mac);

  // Añadir funciones para inicializar el sensor

  Serial.print("[Wi-Fi] Connecting to AP");
  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);

  // Wait for WiFi connection
  while ((wifiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
    Serial.flush();
  }
  Serial.println("\n[Wi-Fi] Connected!");
  setClock();
}

void loop() {

  NetworkClientSecure *client = new NetworkClientSecure;
  if (!client) {
    Serial.println("Unable to create client\n");
    Serial.println("Waiting 10s before the next round...");
    delay(10000);
  }
  client->setCACert(rootCACertificate);
  HTTPClient https;

  // Measurement loop
  while(1) {

    // Configure target server and url
    https.begin(*client, HTTPS_URL);  //HTTP
    https.addHeader("Content-Type", "application/json"); // Sending JSON

    // Añadir funciones para leer valores del sensor

    float temperature = random(-20, 50);
    float pressure = random(90000);

    Serial.print(F("[SENSOR] Temperature: "));
    Serial.print(temperature);
    Serial.println(" ºC");
    Serial.print(F("[SENSOR] Pressure: "));
    Serial.print(pressure);
    Serial.println(" Pa");

    JSONVar payload;
    payload["api_key"] = THINGSPEAK_API_KEY;
    //payload["mac"] = mac;
    payload["field1"] = temperature; // Temp.
    payload["field2"] = pressure; // Pres.

    String payload_str = JSON.stringify(payload);
    Serial.print("Payload: ");
    Serial.println(payload_str);
  
    Serial.print("[HTTP] POST...\n");

    // Start connection and send HTTP header
    int httpCode = https.POST(payload_str);
    if (httpCode <= 0) { // httpCode will be negative on error
      Serial.printf("[HTTP] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
      https.end();
      delay(10000);
      return;
    }
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    // OK response from server
    if (httpCode == HTTP_CODE_OK) {
      Serial.printf("[HTTP] POST OK!\n[HTTP] Response: ");
      Serial.println(https.getString());
    }

    https.end();
    Serial.println("Done!");

    // Delay until next measurement (10s)
    delay(10000);
  }
}
