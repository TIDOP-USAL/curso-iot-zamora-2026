/**
 * Curso IoT 05/2026 Zamora. USAL
 *
 * Práctica 6. Cliente HTTPS y webhook.site. Envío de datos en formato JSON
 * Instala: Arduino_JSON 0.2.0
 *
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

WiFiMulti wifiMulti;

// Variables a modificar
#define WIFI_SSID "CursoIoT_2026"
#define WIFI_PASS "CursoIoT_2026"
#define HTTPS_URL "https://webhook.site/tu-propia-direccion"

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

// ISRG Root X1 certificate
const char *rootCACertificate = R"string_literal(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
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

  JSONVar payload;
  payload["mac"] = mac;
  payload["valor"] = 2;

  String payload_str = JSON.stringify(payload);
  Serial.print("Payload: ");
  Serial.println(payload_str);

  NetworkClientSecure *client = new NetworkClientSecure;
  if (!client) {
    Serial.println("Unable to create client\n");
    Serial.println("Waiting 10s before the next round...");
    delay(10000);
  }
  client->setCACert(rootCACertificate);
  HTTPClient https;

  // Configure target server and url
  https.begin(*client, HTTPS_URL);  //HTTP
  https.addHeader("Content-Type", "application/json"); // Sending JSON

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
    Serial.printf("[HTTP] POST OK!\n");
  }

  https.end();
  Serial.println("Done!");

  while(1) {
    delay(10000);
  }
}
