/**
 * Curso IoT 05/2026 Zamora. USAL
 *
 * Práctica 6. Cliente MQTTS y ThingSpeak. Envío de datos en formato JSON
 * Instala: MQTT 2.5.3         (Joel Gaehwiler)
 *
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>

// Variables a modificar
#define WIFI_SSID "CursoIoT_2026"
#define WIFI_PASS "CursoIoT_2026"
#define MQTT_CLIENT_USERNAME ""
#define MQTT_CLIENT_PASSWORD ""
#define THINGSPEAK_CHANNEL ""
// No modificamos
#define MQTT_BROKER_URL "mqtt3.thingspeak.com"
#define MQTT_BROKER_PORT 8883

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

WiFiClientSecure net;
MQTTClient mqttClient;

unsigned long lastMillis = 0;

void mqttConnect() {

  Serial.print("[MQTT] Connecting...");
  
  while (!mqttClient.connect(MQTT_CLIENT_USERNAME, MQTT_CLIENT_USERNAME, MQTT_CLIENT_PASSWORD)) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\n[MQTT] Connected!");

  bool sub_ok = mqttClient.subscribe("channels/"THINGSPEAK_CHANNEL"/subscribe/fields/+");
  if (sub_ok) {
    Serial.println("[MQTT] Subscribed to topic!");
  } else {
    Serial.println("[MQTT] Failed to subscribe to topic!");
  }
}

void messageReceived(String &topic, String &payload) {
  Serial.println("[MQTT] Incoming: " + topic + " - " + payload);
  //Do not use the client in the callback to publish, subscribe or unsubscribe
}

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

  // Añadir funciones para inicializar el sensor

  Serial.print("[Wi-Fi] Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait for WiFi connection
  while ((WiFi.status() != WL_CONNECTED)) {
    Serial.print(".");
    Serial.flush();
    delay(500);
  }
  Serial.println("\n[Wi-Fi] Connected!");
  setClock();

  net.setCACert(rootCACertificate);
  mqttClient.begin(MQTT_BROKER_URL, MQTT_BROKER_PORT, net);
  mqttClient.onMessage(messageReceived);

  mqttConnect();
}

void sendMessage() {

  // Añadir funciones para leer valores del sensor
  
  float temperature = random(-20, 50);
  float pressure = random(90000);

  Serial.print(F("[SENSOR] Temperature: "));
  Serial.print(temperature);
  Serial.println(" ºC");
  Serial.print(F("[SENSOR] Pressure: "));
  Serial.print(pressure);
  Serial.println(" Pa");

  char payload[200];
  sprintf(payload, "field1=%0.2f&field2=%0.2f", temperature, pressure);
  bool pub_ok = mqttClient.publish(
    "channels/"THINGSPEAK_CHANNEL"/publish",
    (const char*)payload);
  if (pub_ok) {
    Serial.printf("[MQTT] Published OK: \"%s\"\n", (char*)payload);
  } else {
    Serial.println("[MQTT] Failed to publish!");
  }
}

void loop() {
  mqttClient.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!mqttClient.connected()) {
    Serial.println("[MQTT] Disconnected!");
    mqttConnect();
  }

  // Publish a message every ~10 second
  if (millis() - lastMillis > 10000) {
    sendMessage();
    lastMillis = millis();
  }
}
