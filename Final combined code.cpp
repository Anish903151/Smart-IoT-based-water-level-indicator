// === FIREBASE ===
#include <Firebase_ESP_Client.h>
#define API_KEY "YOUR_FIREBASE_API_KEY"
#define DATABASE_URL "https://your-project-id.firebaseio.com/"

// === BLYNK ===
#define BLYNK_TEMPLATE_ID "Your_Template_ID"
#define BLYNK_TEMPLATE_NAME "Smart Water Tank"
#define BLYNK_AUTH_TOKEN "Your_Blynk_Auth"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// === OLED ===
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define WIFI_SSID "Your_SSID"
#define WIFI_PASSWORD "Your_Password"

#define TRIG 5
#define ECHO 18
#define RELAY 4
#define BUZZER 15
#define LED 16

#define TANK_DEPTH_CM 200

Adafruit_SSD1306 display(128, 64, &Wire, -1);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool manualControl = false;

BLYNK_WRITE(V1) {
  manualControl = param.asInt();
  // Write to Firebase manual control (optional)
  Firebase.RTDB.setBool(&fbdo, "/pump_control", manualControl);
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

float measureDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH);
  return duration * 0.034 / 2;
}

void loop() {
  Blynk.run();

  float distance = measureDistance();
  int level = 100 - (distance * 100 / TANK_DEPTH_CM);
  if (level < 0) level = 0;
  if (level > 100) level = 100;

  // OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Water Level: ");
  display.print(level);
  display.println("%");
  display.display();

  // Blynk
  Blynk.virtualWrite(V0, level);

  // Firebase
  Firebase.RTDB.setInt(&fbdo, "/water_level", level);

  // Manual or Auto pump control
  if (manualControl) {
    digitalWrite(RELAY, HIGH);
  } else {
    if (level < 25) {
      digitalWrite(RELAY, HIGH);
      digitalWrite(BUZZER, HIGH);
      digitalWrite(LED, HIGH);
    } else if (level > 90) {
      digitalWrite(RELAY, LOW);
      digitalWrite(BUZZER, HIGH);
      digitalWrite(LED, HIGH);
    } else {
      digitalWrite(RELAY, LOW);
      digitalWrite(BUZZER, LOW);
      digitalWrite(LED, LOW);
    }
  }

  delay(5000);
}
