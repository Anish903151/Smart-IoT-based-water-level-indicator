#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Adafruit_SSD1306.h>

// Wi-Fi
#define WIFI_SSID "Your_SSID"
#define WIFI_PASSWORD "Your_Password"

// Firebase
#define API_KEY "Your_Firebase_API_Key"
#define DATABASE_URL "https://your-project-id.firebaseio.com/" // no quotes after slash

#define USER_EMAIL "test@iot.com"  // dummy user (optional for auth)
#define USER_PASSWORD "12345678"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#define TRIG 5
#define ECHO 18
#define RELAY 4

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(RELAY, OUTPUT);

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

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
  float distance = measureDistance();
  int level = 100 - (distance * 100 / 200);
  if (level < 0) level = 0;
  if (level > 100) level = 100;

  // OLED Display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Level: ");
  display.print(level);
  display.println("%");
  display.display();

  // Push to Firebase
  if (Firebase.RTDB.setInt(&fbdo, "/water_level", level)) {
    Serial.println("Data Sent: " + String(level));
  } else {
    Serial.println("Error: " + fbdo.errorReason());
  }

  delay(5000);
}
