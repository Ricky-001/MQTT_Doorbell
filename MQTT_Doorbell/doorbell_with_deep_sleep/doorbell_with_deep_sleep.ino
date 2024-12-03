/************************************************
* doorbell_with_deep_sleep [ESP32]
* This code uses the PubSubClient library to
* convert an ESP32 into an MQTT client and sends
* specific MQTT messages over the local network 
* on press of a button, which acts as the doorbell
* 
* The ESP32 remains in deep sleep while not in use
* but comes online, connects to Wi-Fi & MQTT
* and sends the specific message to a broker on
* the local network using pre-defined credentials
************************************************/
#include <WiFi.h>
#include <PubSubClient.h>
// https://pubsubclient.knolleary.net/api 
#include "esp_sleep.h"

// Wi-Fi credentials
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";
// MQTT broker details
char* mqtt_server = "192.168.0.100";  // IP address of your MQTT broker
const int mqtt_port = 1883;
const char* topic = "home/doorbell";  // Topic for the doorbell message
// MQTT username and password
const char* mqtt_user = "MQTT_USERID";
const char* mqtt_pass = "MQTT_PASSWORD";

// MQTT and Wi-Fi clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// GPIO pin for the doorbell button
const int switchPin = 4;
const int ledPin = 5;  // GPIO pin for the LED indicator light

// Function to connect to Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.print("[*] INFO: Connecting to Wi-Fi - SSID: ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();  // Record the start time
  const unsigned long timeout = 10000;  // Set timeout duration to 10 seconds (10000 ms)

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");

    // Check if the timeout has been exceeded
    if (millis() - startTime >= timeout) {
      Serial.println("\n[-] ERROR: WiFi_Connection_Timeout: Cannot connect to Wi-Fi.");
      return;  // Exit the function if connection times out
    }
  }

  Serial.println("\n[+] SUCCESS: Connected to Wi-Fi");
  Serial.print("[*] INFO: IP address:\t");
  Serial.println(WiFi.localIP());
}

// Function to reconnect to the MQTT broker
void reconnect() {
  int retryCount = 0;  // Counter to keep track of connection attempts
  const int maxRetries = 2;  // Maximum number of retries allowed

  while (!mqttClient.connected()) {
    if (retryCount >= maxRetries) {
      Serial.println("[!] WARN: Max retries reached. Cannot connect to MQTT broker!");
      return;  // Exit the reconnect function if max retries are reached
    }
    Serial.print("[*] INFO: Connecting to MQTT broker... Attempt ");
    Serial.print(retryCount + 1);
    Serial.print(" of ");
    Serial.println(maxRetries);
    
    if (mqttClient.connect("ESP32Doorbell", mqtt_user, mqtt_pass)) {
      Serial.println("[+] SUCCESS: Connected to MQTT broker!");
    } else {
      Serial.print("[-] ERROR: Connection failed with State ");
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

void ledFeedback() {
  // disable GPIO hold - to change state
    gpio_hold_dis(GPIO_NUM_5);
    // LED blink sequence
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(300);
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
    delay(500);
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
}

// Task handle for LED feedback
TaskHandle_t ledTaskHandle = NULL;

void ledFeedbackTask(void* parameter) {
  Serial.println("[**] ASYNC_INFO: Task started!");
  gpio_hold_dis(GPIO_NUM_5);
  const int ledPin = 5;  // LED GPIO pin
  pinMode(ledPin, OUTPUT);

  // LED blink sequence
  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);
  delay(300);
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(500);
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);

  // Delete the task after completion
  vTaskDelete(NULL);
  Serial.println("[**] ASYNC_INFO: Task completed!");
}


void doorbellPressed() {
  // Start the LED feedback task
  if (ledTaskHandle == NULL) {  // Check if the task is already running
    xTaskCreate(
      ledFeedbackTask,    // Task function
      "LED Feedback Task", // Task name
      1000,                // Stack size
      NULL,                // Task parameters
      1,                   // Priority
      &ledTaskHandle       // Task handle
    );
  }
    // Connect to Wi-Fi
    setup_wifi();
    // connect to MQTT
    connectToMQTT();

    if (mqttClient.connected() && WiFi.status() == WL_CONNECTED) {
      // Send MQTT message
      mqttClient.publish(topic, "Ding Dong! Someone is at the door.");
      Serial.println("[+] SUCCESS: Doorbell pressed, sending MQTT message.");
      delay(1000);
    }
    else if (!mqttClient.connected() && WiFi.status() != WL_CONNECTED) {
      Serial.println("[-] ERROR: Could not connect to MQTT broker. Please ensure Broker is running");
      Serial.println("[-] ERROR: Could not connect to Wi-Fi. Please check SSID/ Password");
      Serial.println("[!!] WARN: Doorbell will NOT ring!");
    }
    else if (!mqttClient.connected()) {
      Serial.println("[-] ERROR: Could not connect to MQTT broker. Please ensure Broker is running");
      Serial.println("[!] WARN: Doorbell will NOT ring!");
    }
    else if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[-] ERROR: Could not connect to Wi-Fi. Please check SSID/ Password");
      Serial.println("[!] WARN: Doorbell will NOT ring!");
    }
    
    // Disconnect from Wi-Fi and MQTT to save power before going to sleep
    mqttClient.disconnect();
    WiFi.disconnect(true);
}

void setup() {
  // Set up serial communication and the button pin
  Serial.begin(115200);
  pinMode(switchPin, INPUT_PULLUP);  // Using INPUT_PULLUP for button
  pinMode(ledPin, OUTPUT);  // Set LED pin as output
  digitalWrite(ledPin, LOW);  // Turn off LED initially

// Configure the ESP32 to wake up from deep sleep when button is pressed
  esp_sleep_enable_ext0_wakeup((gpio_num_t)switchPin, LOW);  // Wake up when button is pressed (LOW)

  // Check if the ESP32 woke up due to a touch
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("[*] INFO: Touch input received, waking up from deep sleep...");
    doorbellPressed();  // Call the doorbell function 
  } 
}

void loop() {
  Serial.println("[**] LOOP_INFO: ESP32 ready, touch to ring the doorbell.");
  digitalWrite(ledPin, LOW);
  // Put ESP32 to sleep
  Serial.println("[#] INFO: Going to deep sleep...");
  delay(2000);  // Small delay to let serial messages be printed before sleep
  // enable GPIO hold state at LOW
  gpio_hold_en(GPIO_NUM_5);
  gpio_deep_sleep_hold_en();
  esp_deep_sleep_start();  // Enter deep sleep
}
