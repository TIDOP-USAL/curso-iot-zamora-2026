/**
 * Curso IoT 05/2026 Zamora. USAL
 *
 * Práctica 9. End Device LoRaWAN. Conexión a TTN.
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

void loop() {
  delay(1000UL);
}
