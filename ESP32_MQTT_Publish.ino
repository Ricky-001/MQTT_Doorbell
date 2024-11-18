#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_sleep.h"

// Wi-Fi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASS";

// MQTT broker details
char* mqtt_server = "192.168.0.100";  // IP address of your MQTT broker
const int mqtt_port = 1883;
const char* topic = "test/topic";  // Topic for the doorbell message

// MQTT username and password
const char* mqtt_user = "root";
const char* mqtt_pass = "toor";

// MQTT and Wi-Fi clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// GPIO pin for the doorbell button
const int ledPin = 5;  // GPIO pin for the LED indicator light
// Touch threshold (adjust based on sensitivity)
const int touchThreshold = 20;  // Lower values are more sensitive
bool touchDetected = false;

// Function to connect to Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.print("INFO: Connecting to Wi-Fi - SSID: ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();  // Record the start time
  const unsigned long timeout = 10000;  // Set timeout duration to 10 seconds (10000 ms)

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");

    // Check if the timeout has been exceeded
    if (millis() - startTime >= timeout) {
      Serial.println("\nERROR: WiFi_Connection_Timeout: Cannot connect to Wi-Fi.");
      return;  // Exit the function if connection times out
    }
  }

  Serial.println("\nSUCCESS: Connected to Wi-Fi");
}

// Function to reconnect to the MQTT broker
void reconnect() {
  int retryCount = 0;  // Counter to keep track of connection attempts
  const int maxRetries = 5;  // Maximum number of retries allowed

  while (!mqttClient.connected()) {
    if (retryCount >= maxRetries) {
      Serial.println("WARN: Max retries reached. Cannot connect to MQTT broker!");
      return;  // Exit the reconnect function if max retries are reached
    }
    Serial.print("INFO: Connecting to MQTT broker... Attempt ");
    Serial.print(retryCount + 1);
    Serial.print(" of ");
    Serial.println(maxRetries);
    
    if (mqttClient.connect("ESP32Doorbell", mqtt_user, mqtt_pass)) {
      Serial.println("SUCCESS: Connected to MQTT broker!");
    } else {
      Serial.print("ERROR: Connection failed with State ");
      Serial.println(mqttClient.state());
      retryCount++;  // Increment the retry count
      delay(2000);
    }
  }
}

// Function to connect to MQTT Broker
void connectToMQTT() {
  mqttClient.setServer(mqtt_server, mqtt_port);
  // Ensure connection to MQTT broker
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
}

void doorbellPressed() {
    // Connect to Wi-Fi & MQTT
    setup_wifi();
    connectToMQTT();

    if (mqttClient.connected() && WiFi.status() == WL_CONNECTED) {
      // Send MQTT message
      mqttClient.publish(topic, "Ding Dong! Someone is at the door.");
      Serial.println("SUCCESS: Doorbell pressed, sending MQTT message.");
    }
    else if (!mqttClient.connected() && WiFi.status() != WL_CONNECTED) {
      Serial.println("ERROR: Could not connect to MQTT broker. Please ensure Broker is running");
      Serial.println("ERROR: Could not connect to Wi-Fi. Please check SSID/ Password");
      Serial.println("WARN: Doorbell will NOT ring!");
    }
    else if (!mqttClient.connected()) {
      Serial.println("ERROR: Could not connect to MQTT broker. Please ensure Broker is running");
      Serial.println("WARN: Doorbell will NOT ring!");
    }
    else if (WiFi.status() != WL_CONNECTED) {
      Serial.println("ERROR: Could not connect to Wi-Fi. Please check SSID/ Password");
      Serial.println("WARN: Doorbell will NOT ring!");
    }
    
    // Disconnect from Wi-Fi and MQTT to save power before going to sleep
    mqttClient.disconnect();
    WiFi.disconnect(true);
}

void isDoorbellPressed() {
  touchDetected = true;
}

void setup() {
  // Set up serial communication and the button pin
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);  // Set LED pin as output
  digitalWrite(ledPin, LOW);  // Turn off LED initially

  // Initialize touch pin
  touchAttachInterrupt(T0, isDoorbellPressed, touchThreshold);
  // Configure deep sleep to wake up on touch (if using deep sleep mode)
  esp_sleep_enable_touchpad_wakeup();

  // Check if the ESP32 woke up due to a touch
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD) {
    Serial.println("INFO: Touch input received, waking up from deep sleep...");
    doorbellPressed();  // Call the doorbell function if waking up due to touch  
    touchDetected = false;
  } 
}

void loop() {
  if (touchDetected) {
    touchDetected = false;
  }
  Serial.println("LOOP_INFO: ESP32 ready, touch to ring the doorbell.");
  // Put ESP32 to sleep
  Serial.println("INFO: Going to deep sleep...");
  delay(1000);  // Small delay to let serial messages be printed before sleep
  esp_deep_sleep_start();  // Enter deep sleep
}
