#include "Arduino.h"
#include "LoRaWan_APP.h"
#include "Wire.h"
#include "HT_SSD1306Wire.h"   // <-- provides fonts + TEXT_ALIGN_* + SSD1306Wire type

// The Heltec library defines the OLED object in its .cpp file.
// We declare it here so the compiler knows it exists.
extern SSD1306Wire display;

// ---- LoRa Radio Settings (must match RX) ----
#define RF_FREQUENCY            915000000  // Hz
#define TX_OUTPUT_POWER         14         // dBm
#define LORA_BANDWIDTH          0          // 0=125kHz
#define LORA_SPREADING_FACTOR   7          // SF7..SF12
#define LORA_CODINGRATE         1          // 1=4/5
#define LORA_PREAMBLE_LENGTH    8
#define LORA_IQ_INVERSION       false

static RadioEvents_t RadioEvents;

static uint8_t txBuf[64];
static uint32_t lastSendMs = 0;
static uint32_t counter = 0;

void OnTxDone(void) {
  Serial.println("TX done");
  Radio.Sleep();
}

void OnTxTimeout(void) {
  Serial.println("TX timeout");
  Radio.Sleep();
}

static void oledPrint2(const String &l1, const String &l2) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, l1);
  display.drawString(0, 12, l2);
  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // Board init
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  // Turn on OLED power (Vext)
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  delay(50);

  // Init OLED
  display.init();
  oledPrint2("LoRa TX", "Booting...");

  // Radio callbacks
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;

  // Init radio + configure
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetTxConfig(
    MODEM_LORA,
    TX_OUTPUT_POWER,
    0,
    LORA_BANDWIDTH,
    LORA_SPREADING_FACTOR,
    LORA_CODINGRATE,
    LORA_PREAMBLE_LENGTH,
    false,   // fixed length payload
    true,    // CRC on
    0, 0,
    LORA_IQ_INVERSION,
    3000
  );

  Radio.Sleep();

  Serial.println("LoRa TX ready");
  oledPrint2("LoRa TX ready", "Sending every 60s");
}

void loop() {
  uint32_t now = millis();

  if (now - lastSendMs >= 60000UL) {
    lastSendMs = now;
    counter++;

    int n = snprintf((char*)txBuf, sizeof(txBuf), "t=%lu ms cnt=%lu",
                     (unsigned long)now, (unsigned long)counter);

    Serial.printf("Sending: %s\n", txBuf);
    oledPrint2("Sending:", String((char*)txBuf));

    Radio.SetChannel(RF_FREQUENCY);
    Radio.Send(txBuf, (uint8_t)n);
  }

  Mcu.timerhandler();
  Radio.IrqProcess();
}
