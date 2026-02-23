/*
  Heltec WiFi LoRa 32 (V3) - RX (ESP32-S3 + SX1262)
  Receives LoRa packets and prints them to USB Serial (laptop).
*/

#include <heltec_unofficial.h>

// Must match TX exactly
static const float LORA_FREQ_MHZ = 915.0;  // <-- CHANGE if needed to match TX

void setup() {
  heltec_setup();

  int state = radio.begin();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("LoRa radio.begin() failed, code=%d (%s)\n",
                  state, radiolib_result_string(state));
    while (true) { heltec_loop(); }
  }

  state = radio.setFrequency(LORA_FREQ_MHZ);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("radio.setFrequency(%.1f) failed, code=%d (%s)\n",
                  LORA_FREQ_MHZ, state, radiolib_result_string(state));
    while (true) { heltec_loop(); }
  }

  // OPTIONAL LoRa params (must match TX if you change them)
  // radio.setSpreadingFactor(7);
  // radio.setBandwidth(125.0);
  // radio.setCodingRate(5);

  Serial.println("RX ready. Waiting for packets...");
}

void loop() {
  heltec_loop();

  String msg;
  int state = radio.receive(msg);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.print("Received: ");
    Serial.println(msg);
  } else if (state != RADIOLIB_ERR_RX_TIMEOUT && state != RADIOLIB_ERR_NONE) {
    Serial.printf("RX error, code=%d (%s)\n", state, radiolib_result_string(state));
  }

  // Small delay to reduce spam; receive() blocks internally depending on config
  delay(10);
}
