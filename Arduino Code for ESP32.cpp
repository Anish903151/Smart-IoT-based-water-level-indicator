#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>

#define TRIG 5
#define ECHO 18
#define RELAY 4
#define BUZZER 15
#define LED 16

#define OLED_SDA 21
#define OLED_SCL 22

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Tank dimensions
const int maxDistance = 200;  // in cm (depth of tank)
const int minLevel = 25;      // alert level
const int maxLevel = 90;      // full tank

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(RELAY, LOW); // Pump OFF initially

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  WiFi.begin(ssid, password);
  display.print("Connecting to WiFi...");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("WiFi Connected!");
  display.display();
  delay(1000);
}

float measureDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH);
  float distance = duration * 0.034 / 2;
  return distance;
}

void loop() {
  float distance = measureDistance();
  int levelPercent = 100 - (distance * 100 / maxDistance);
  if (levelPercent < 0) levelPercent = 0;
  if (levelPercent > 100) levelPercent = 100;

  // Display on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Level: ");
  display.print(levelPercent);
  display.println("%");
  display.display();

  // Control Pump and Alerts
  if (levelPercent < minLevel) {
    digitalWrite(RELAY, HIGH); // Turn ON pump
    digitalWrite(BUZZER, HIGH);
    digitalWrite(LED, HIGH);
    sendAlert("LOW"); // cloud alert
  } else if (levelPercent > maxLevel) {
    digitalWrite(RELAY, LOW);  // Turn OFF pump
    digitalWrite(BUZZER, HIGH);
    digitalWrite(LED, HIGH);
    sendAlert("OVERFLOW");
  } else {
    digitalWrite(RELAY, LOW); // Normal
    digitalWrite(BUZZER, LOW);
    digitalWrite(LED, LOW);
  }

  delay(5000);
}

void sendAlert(String status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://maker.ifttt.com/trigger/" + status + "/with/key/YOUR_IFTTT_KEY";
    http.begin(url);
    int httpCode = http.GET();
    http.end();
  }
}
