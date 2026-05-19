/**
 * Curso IoT 05/2026 Zamora. USAL
 *
 * Práctica 9. End Device LoRaWAN. Envío de datos del sensor.
 * Instala: RadioLib 7.6.0         (Jan Gromes)
 *
 */

// This example REQUIRES you to check "Resets DevNonces" on TTN.

// Variables a modificar:
#define RADIOLIB_LORAWAN_JOIN_EUI  0x0000000000000000
#define RADIOLIB_LORAWAN_DEV_EUI   0x----------------
#define RADIOLIB_LORAWAN_APP_KEY   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#define RADIOLIB_LORAWAN_NWK_KEY   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

#include <esp_wifi.h>
#include "config.h"

// How often to send an uplink
const uint32_t uplinkIntervalSeconds = 10UL;

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

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  while(!Serial);
  delay(1000);
  Serial.println();

  char mac[20];
  Serial.print("XIAO MAC ");
  getMacAddress((char *)&mac);
  Serial.printf("%s\n", mac);

  // Añadir funciones para inicializar el sensor

  Serial.println(F("\n[LORAWAN] Setup ... "));

  Serial.println(F("[LORAWAN] Radio init ..."));
  int16_t state = radio.begin();
  debug(state != RADIOLIB_ERR_NONE, F("[LORAWAN] Initialise radio failed"), state, true);
  state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
  debug(state != RADIOLIB_ERR_NONE, F("[LORAWAN] Initialise node failed"), state, true);

  Serial.println(F("[LORAWAN] Join TTN"));
  state = node.activateOTAA();
  debug(state != RADIOLIB_LORAWAN_NEW_SESSION, F("[LORAWAN] Join failed"), state, true);

  Serial.println(F("[LORAWAN] Join OK. Ready!\n"));
}

typedef union {
    uint32_t intValue;
    uint8_t bytes[sizeof(uint32_t)];
} uint32tToBytes_t;

void loop() {
  digitalWrite(LED_BUILTIN, LOW);

  // Añadir funciones para leer valores del sensor

  float temperature_raw = random(50);
  float pressure_raw = random(90000);

  Serial.print(F("[SENSOR] Temperature: "));
  Serial.print(temperature_raw);
  Serial.println(" ºC");
  Serial.print(F("[SENSOR] Pressure: "));
  Serial.print(pressure_raw);
  Serial.println(" Pa");

  int16_t temperature = (int16_t)(temperature_raw * 100); // ºC (2 decimales)
  int32_t pressure = (uint32_t)(pressure_raw * 100); // Pa (2 decimales)
  // Un int16_t ocupa 2 bytes. Un int32_t ocupa 4 bytes.

  // Build payload byte array
  uint8_t uplinkPayload[6];
  uplinkPayload[0] = highByte(temperature);
  uplinkPayload[1] = lowByte(temperature);
  // Pressure into bytes
  uint32tToBytes_t p; p.intValue = pressure;
  uplinkPayload[2] = p.bytes[3];
  uplinkPayload[3] = p.bytes[2];
  uplinkPayload[4] = p.bytes[1];
  uplinkPayload[5] = p.bytes[0];
  
  // Perform an uplink
  Serial.println(F("[LORAWAN] Sending uplink"));
  int16_t state = node.sendReceive(uplinkPayload, sizeof(uplinkPayload));    
  debug(state < RADIOLIB_ERR_NONE, F("[LORAWAN] Error in sendReceive"), state, true);
  digitalWrite(LED_BUILTIN, HIGH);

  if(state > 0) {
    Serial.println(F("[LORAWAN] Received a downlink"));
  } else {
    Serial.println(F("[LORAWAN] No downlink received"));
  }

  Serial.print(F("[SENSOR] Next uplink in "));
  Serial.print(uplinkIntervalSeconds);
  Serial.println(F(" seconds\n"));
  
  // Wait until next uplink - Legal & TTN constraints
  delay(uplinkIntervalSeconds * 1000UL);
}
