#include <Arduino.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "time.h"

// WiFi Credentials
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// Firebase Things Database
#define DATABASE_URL "https://upcpre202402si572sw74-default-rtdb.firebaseio.com/.json"

HTTPClient client;


// Pin Definition
#define TRIGGER_PIN 5
#define ECHO_PIN 18
#define GREEN_LED 4
#define RED_LED 2

// LCD Setup
LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 16, 2);

// Variables section
int previous = 0;
String tempText = "";
String payload = "";
String sensorID = "HC001";
char timeStringBuffer[20];
String greenLedOn;
String redLedOn;
String safeDistanceOn;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Kick-off message
  LCD.init();
  LCD.backlight();
  LCD.setCursor(0, 0);
  LCD.print("Connecting to ");
  LCD.setCursor(0, 1);
  LCD.print("WiFi ");

  // WiFi Connection Setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.print(WiFi.localIP());
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.println("Online");
  delay(500);

  // Firebase Connection Setup
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.println("Connecting to");
  LCD.setCursor(0, 1);
  LCD.println("Firebase...");
  Serial.println("connecting...");

  client.begin(DATABASE_URL);
  int httpResponseCode = client.GET();
  if (httpResponseCode > 0) {
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.println("Connected");
    Serial.println("connected, Firebase payload:");
    payload = client.getString();
    Serial.println(payload);
  } 

  // Distance Sensor Setup
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // LEDs setup
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  Serial.println("Green = Safe");
  Serial.println("Red   = Unsafe");

  for (int i = 0; i < 5; i++) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, HIGH);
    delay(200);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    delay(200);
  }

  // Network Time Provider Setup
  configTime(-9000, -9000, "1.south-america.pool.ntp.org");
}

void loop() {
  
  long distancePulse, distanceMetric;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, LOW);
  distancePulse = pulseIn(ECHO_PIN, HIGH);
  distanceMetric = (distancePulse/2)/29.1;

  if (previous != distanceMetric) {
    if (distanceMetric > 200 || distanceMetric < 0) {
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      tempText = "Safe distance: ";
      greenLedOn = "true";
      redLedOn =  "false";
      safeDistanceOn = "true";
    } else {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      tempText = "Unsafe distance: ";
      greenLedOn = "false";
      redLedOn =  "true";
      safeDistanceOn = "false";
    }
  
    struct tm timeInfo;
    if (!getLocalTime(&timeInfo)) {
      Serial.println("Time Error");
    }
    strftime(timeStringBuffer, sizeof(timeStringBuffer), "%d/%m/%Y %H:%M", &timeInfo);
    Serial.println(String(timeStringBuffer));

    client.PATCH("{\"Status/Sensors/time\":\"" + String(timeStringBuffer) + "\"}");
    client.PATCH("{\"Status/Led/greenLed\":" + greenLedOn + "}");

  }
  previous = distanceMetric;
  delay(500);
}
