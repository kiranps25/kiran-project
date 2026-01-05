#include "Arduino.h"
#include "LoRa_E32.h"

// Buzzer pin
#define BUZZER_PIN 27

LoRa_E32 e32ttl(&Serial2, 18, 4, 5);  // AUX=18, M0=4, M1=5

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  
  // Initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  delay(500);
  e32ttl.begin();
  delay(100);
  
  Serial.println("=============================");
  Serial.println("LoRa Chat Master - Ready");
  Serial.println("Type and press Enter to send");
  Serial.println("=============================");
  
  // Startup beep
  beep(150);
  delay(100);
  beep(150);
}

void loop() {
  // PC to LoRa - Send messages
  if (Serial.available()) {
    String outgoing = Serial.readStringUntil('\n');
    outgoing.trim();
    
    if (outgoing.length() > 0) {
      Serial2.println(outgoing);
      Serial.print("[SENT] ");
      Serial.println(outgoing);
      
      // Short beep when sending
      beep(50);
      
      delay(50);
    }
  }
  
  // LoRa to PC - Receive messages
  if (Serial2.available()) {
    String received = Serial2.readStringUntil('\n');
    received.trim();
    
    if (received.length() > 0) {
      Serial.print("[RECEIVED] ");
      Serial.println(received);
      
      // 3 BEEPS when receiving message
      beep(200);
      delay(150);
      beep(200);
      delay(150);
      beep(200);
    }
  }
  
  delay(10);
}

// Beep function
void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}
