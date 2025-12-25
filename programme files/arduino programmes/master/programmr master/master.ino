#include "Arduino.h"
#include "LoRa_E32.h"

#define RXD2 5
#define TXD2 17
#define M0_PIN 4
#define M1_PIN 16
#define AUX_PIN 18

LoRa_E32 e32ttl(&Serial2, AUX_PIN, M0_PIN, M1_PIN);

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(500);
  
  e32ttl.begin();
  
  Serial.println("LoRa Master Ready");
}

void loop() {
  // Check for incoming data from PC (Flutter app via USB Serial)
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    message.trim();
    
    if (message.length() > 0) {
      // Send via LoRa to Slave
      ResponseStatus rs = e32ttl.sendMessage(message);
      
      if (rs.code == 1) {
        Serial.println("[SENT] " + message);
      } else {
        Serial.println("[ERROR] Failed to send");
      }
    }
  }
  
  // Check for incoming LoRa messages FROM SLAVE
  if (e32ttl.available() > 0) {
    ResponseContainer rc = e32ttl.receiveMessage();
    
    if (rc.status.code == 1) {
      String received = rc.data;
      
      // CRITICAL: Print with [RECEIVED] tag so Flutter can identify it
      Serial.println("[RECEIVED] " + received);
    }
  }
  
  delay(10);
}
