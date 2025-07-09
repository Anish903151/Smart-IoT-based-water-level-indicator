
#define BLYNK_TEMPLATE_ID "Your_Template_ID"
#define BLYNK_TEMPLATE_NAME "Smart Water Tank"
#define BLYNK_AUTH_TOKEN "Your_Auth_Token"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_SSD1306.h>

char ssid[] = "Your_WiFi_Name";
char pass[] = "Your_WiFi_Password";

#define TRIG 5
#define ECHO 18
#define RELAY 4
#define BUZZER 15
#define LED 16

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Blynk virtual pins
#define V_LEVEL V0
#define V_MANUAL_CONTROL V1

bool manualPumpControl = false;

BLYNK_WRITE(V_MANUAL_CONTROL) {
  manualPumpControl = param.asInt();
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  Wire.begin(21, 22); // SDA, SCL for OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
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
  int level = 100 - (distance * 100 / 200);  // assuming 200 cm tank depth
  if (level < 0) level = 0;
  if (level > 100) level = 100;

  Blynk.virtualWrite(V_LEVEL, level);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Level: ");
  display.print(level);
  display.println("%");
  display.display();

  if (manualPumpControl) {
    digitalWrite(RELAY, HIGH);  // manual ON
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
