#include "Arduino.h"
#include "LoRaWan_APP.h"
#include "Wire.h"
#include "HT_SSD1306Wire.h"

// Heltec library provides the global OLED object when using LoRaWan_APP
extern SSD1306Wire display;

// ---- Must match the Sender ----
#define RF_FREQUENCY            915000000  // Hz
#define LORA_BANDWIDTH          0          // 0=125kHz
#define LORA_SPREADING_FACTOR   7          // SF7..SF12
#define LORA_CODINGRATE         1          // 1=4/5
#define LORA_PREAMBLE_LENGTH    8
#define LORA_IQ_INVERSION       false

static RadioEvents_t RadioEvents;

static void oledPrint3(const String &l1, const String &l2, const String &l3) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, l1);
  display.drawString(0, 12, l2);
  display.drawString(0, 24, l3);
  display.display();
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  Radio.Sleep();

  // Convert payload to printable String (safe for non-null-terminated buffers)
  String msg = "";
  for (uint16_t i = 0; i < size; i++) {
    msg += (char)payload[i];
  }

  Serial.printf("RX (%u bytes): %s | RSSI=%d dBm | SNR=%d dB\n",
                (unsigned)size, msg.c_str(), (int)rssi, (int)snr);

  oledPrint3("Received:", msg, "RSSI " + String(rssi) + " / SNR " + String(snr));

  // Go back to continuous receive
  Radio.Rx(0);
}

void OnRxTimeout(void) {
  // Keep listening
  Radio.Rx(0);
}

void OnRxError(void) {
  // Keep listening
  Radio.Rx(0);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // Board init
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  // OLED power on (Vext)
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  delay(50);

  // OLED init
  display.init();
  oledPrint3("LoRa RX", "Booting...", "");

  // Radio callbacks
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  // Init radio
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);

  // Configure LoRa modem RX
  Radio.SetRxConfig(
    MODEM_LORA,
    LORA_BANDWIDTH,
    LORA_SPREADING_FACTOR,
    LORA_CODINGRATE,
    0,                    // AFC bandwidth (FSK only)
    LORA_PREAMBLE_LENGTH,
    0,                    // symbol timeout
    false,                // fixed length payload
    0,                    // payload length (if fixed)
    true,                 // CRC on
    0, 0,                 // freq hop period, hop on
    LORA_IQ_INVERSION,
    true                  // continuous RX
  );

  Serial.println("LoRa RX ready (listening)");
  oledPrint3("LoRa RX ready", "Listening @ 915MHz", "SF7 BW125 CR4/5");

  // Start receiving continuously
  Radio.Rx(0);
}

void loop() {
  // Service the radio stack
  Mcu.timerhandler();
  Radio.IrqProcess();
}