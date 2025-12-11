// SLAVE.ino
// ESP32-WROOM + E32 433MHz
// Sends messages to master (MST) and can ask master to forward to another slave via "B02:Hello"

#define LORA_SERIAL  Serial1
#define LORA_RX_PIN  16
#define LORA_TX_PIN  17
#define LORA_M0_PIN  18
#define LORA_M1_PIN  19
#define LORA_AUX_PIN 4

const char* NODE_ID   = "B01";   // <<< CHANGE this per slave: "A01", "B01", "B02", etc.
const char* MASTER_ID = "MST";

String pcBuffer   = "";
String loraBuffer = "";

void setupPins() {
  pinMode(LORA_M0_PIN, OUTPUT);
  pinMode(LORA_M1_PIN, OUTPUT);
  pinMode(LORA_AUX_PIN, INPUT);

  digitalWrite(LORA_M0_PIN, LOW); // normal mode
  digitalWrite(LORA_M1_PIN, LOW);
}

void waitAux() {
  unsigned long start = millis();
  while (digitalRead(LORA_AUX_PIN) == LOW) {
    if (millis() - start > 1000) break;
  }
}

void setup() {
  Serial.begin(115200); 
  LORA_SERIAL.begin(9600, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
  setupPins();
  delay(500);

  Serial.print("SLAVE started, ID = ");
  Serial.println(NODE_ID);
  Serial.println("Type on Serial Monitor:");
  Serial.println("  text              -> sends to MST");
  Serial.println("  B02:Hello there   -> ask MST to forward to B02");
}

void sendLoRaLine(const String& line) {
  waitAux();
  LORA_SERIAL.print(line);
  LORA_SERIAL.print('\n');
  Serial.print("[LoRa TX] ");
  Serial.println(line);
}

// Handle line from PC Serial
void handlePcLine(const String& line) {
  String s = line;
  s.trim();
  if (s.length() == 0) return;

  String payload;

  int colonIndex = s.indexOf(':');
  if (colonIndex > 0) {
    // Format: TOID:message  -> forward request
    String toId = s.substring(0, colonIndex);
    String msg  = s.substring(colonIndex + 1);
    toId.trim();
    msg.trim();
    if (toId.length() == 0 || msg.length() == 0) {
      Serial.println("Bad format. Example: B02:Hello there");
      return;
    }
    payload = "TO=" + toId + ";" + msg;
  } else {
    // No colon: send directly to master only
    payload = s; // plain message to MST
  }

  // Build packet: SRC|DST|TYPE|PAYLOAD
  String packet = String(NODE_ID) + "|" + MASTER_ID + "|MSG|" + payload;
  sendLoRaLine(packet);
}

// Handle line from LoRa
void handleLoRaLine(const String& line) {
  Serial.print("[LoRa RX raw] ");
  Serial.println(line);

  int p1 = line.indexOf('|');
  int p2 = line.indexOf('|', p1 + 1);
  int p3 = line.indexOf('|', p2 + 1);
  if (p1 < 0 || p2 < 0 || p3 < 0) {
    Serial.println("Malformed packet");
    return;
  }

  String src     = line.substring(0, p1);
  String dst     = line.substring(p1 + 1, p2);
  String type    = line.substring(p2 + 1, p3);
  String payload = line.substring(p3 + 1);

  src.trim(); dst.trim(); type.trim(); payload.trim();

  if (dst == NODE_ID && type == "MSG") {
    Serial.print("MESSAGE FROM ");
    Serial.print(src);
    Serial.print(": ");
    Serial.println(payload);
  } else {
    // Not for this node -> ignore
  }
}

void loop() {
  // 1) Read from PC Serial
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (pcBuffer.length() > 0) {
        handlePcLine(pcBuffer);
        pcBuffer = "";
      }
    } else {
      pcBuffer += c;
    }
  }

  // 2) Read from LoRa
  while (LORA_SERIAL.available()) {
    char c = LORA_SERIAL.read();
    if (c == '\n' || c == '\r') {
      if (loraBuffer.length() > 0) {
        handleLoRaLine(loraBuffer);
        loraBuffer = "";
      }
    } else {
      loraBuffer += c;
    }
  }
}
