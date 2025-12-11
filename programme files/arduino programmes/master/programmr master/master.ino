// MASTER_BRIDGE.ino
// ESP32-WROOM + E32 433MHz
// Acts as a transparent USB <-> LoRa bridge for the Python master GUI

#define LORA_SERIAL  Serial1
#define LORA_RX_PIN  16   // E32 TXD -> ESP32 RX
#define LORA_TX_PIN  17   // E32 RXD -> ESP32 TX
#define LORA_M0_PIN  18
#define LORA_M1_PIN  19
#define LORA_AUX_PIN 4

void setupPins() {
  pinMode(LORA_M0_PIN, OUTPUT);
  pinMode(LORA_M1_PIN, OUTPUT);
  pinMode(LORA_AUX_PIN, INPUT);

  // Normal transparent mode: M0 = 0, M1 = 0
  digitalWrite(LORA_M0_PIN, LOW);
  digitalWrite(LORA_M1_PIN, LOW);
}

void waitAux() {
  // Optional: wait until module is ready (AUX HIGH)
  unsigned long start = millis();
  while (digitalRead(LORA_AUX_PIN) == LOW) {
    if (millis() - start > 1000) break; // timeout safeguard
  }
}

void setup() {
  Serial.begin(115200);   // PC side (USB)
  LORA_SERIAL.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN); // E32 side
  setupPins();
  delay(500);
  Serial.println("ESP32 LoRa bridge ready");
}

void loop() {
  // PC -> LoRa
  while (Serial.available()) {
    char c = Serial.read();
    waitAux();
    LORA_SERIAL.write(c);
  }

  // LoRa -> PC
  while (LORA_SERIAL.available()) {
    char c = LORA_SERIAL.read();
    Serial.write(c);
  }
}
