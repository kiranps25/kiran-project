#include "Arduino.h"
#include "LoRa_E32.h"
#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// LoRa E32 pins
#define RXD2 16
#define TXD2 17
#define M0_PIN 4
#define M1_PIN 5
#define AUX_PIN 18

// OLED Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 4x4 Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {26, 25, 33, 32};
byte colPins[COLS] = {13, 12, 14, 27};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LoRa E32 module
LoRa_E32 e32ttl(&Serial2, AUX_PIN, M0_PIN, M1_PIN);

String inputBuffer = "";
String lastReceived = "";

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(500);
  
  // Initialize I2C and OLED
  Wire.begin(21, 22);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("LoRa Slave Init...");
  display.display();
  delay(1000);
  
  // Initialize E32 LoRa
  e32ttl.begin();
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("LoRa Chat Ready");
  display.println("----------------");
  display.display();
  
  Serial.println("Slave ready");
}

void loop() {
  // Read keyboard input
  char key = keypad.getKey();
  
  if (key) {
    if (key == 'A') {
      // Send message on 'A' press
      if (inputBuffer.length() > 0) {
        sendMessage(inputBuffer);
        inputBuffer = "";
        updateDisplay();
      }
    } else if (key == '*') {
      // Clear buffer on '*' press
      inputBuffer = "";
      updateDisplay();
    } else if (key == 'D') {
      // Backspace on 'D' press
      if (inputBuffer.length() > 0) {
        inputBuffer.remove(inputBuffer.length() - 1);
        updateDisplay();
      }
    } else {
      // Add character to buffer
      inputBuffer += key;
      updateDisplay();
    }
  }
  
  // Check for incoming LoRa messages
  if (e32ttl.available() > 0) {
    ResponseContainer rc = e32ttl.receiveMessage();
    
    if (rc.status.code == 1) {
      lastReceived = rc.data;
      Serial.println("[RECEIVED] " + lastReceived);
      updateDisplay();
    }
  }
}

void sendMessage(String msg) {
  ResponseStatus rs = e32ttl.sendMessage(msg);
  
  if (rs.code == 1) {
    Serial.println("[SENT] " + msg);
  } else {
    Serial.println("[ERROR] Send failed");
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  
  // Header
  display.println("LoRa Chat (Slave)");
  display.println("----------------");
  
  // Last received message
  display.print("RX: ");
  if (lastReceived.length() > 18) {
    display.println(lastReceived.substring(0, 18));
  } else {
    display.println(lastReceived);
  }
  
  display.println("----------------");
  
  // Input buffer
  display.print("TX: ");
  if (inputBuffer.length() > 18) {
    display.println(inputBuffer.substring(inputBuffer.length() - 18));
  } else {
    display.println(inputBuffer);
  }
  
  display.println("");
  display.println("A Send * Clear");
  
  display.display();
}
