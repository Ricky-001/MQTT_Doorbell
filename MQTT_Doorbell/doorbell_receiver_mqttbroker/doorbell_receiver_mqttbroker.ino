/************************************************
* doorbell_receiver_mqttbroker [ESP8266]
* This code uses the sMQTTBroker library to convert
* an ESP8266 NodeMCU into an MQTT broker on a local 
* network, connected by Wi-Fi to enable MQTT comms.
* 
* The ESP8266 will act as the MQTT broker, while
* also processing the incoming message for the 
* doorbell press, and act as the receiver unit
* of the doorbell, playing audio-visual feedback.
* 
* The audio is played using the ESP8266Audio libraries
* and by directly feeding the audio data into 
* the doorbell.h header file as a hex code.
* The audio file should be in WAV format, 16-bit PCM
* before converting the contents into Hex and
* pasting the contents into the doorbell.h
************************************************/
#include <Arduino.h>
#include <sMQTTBroker.h>
#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2SNoDAC.h"
#include "doorbell.h"

AudioGeneratorWAV *wav;
AudioFileSourcePROGMEM *file;
AudioOutputI2SNoDAC *out;

const char* MQTT_CLIENT_USER = "MQTT_USERID"; // username for mqtt clients. Set your own value here.
const char* MQTT_CLIENT_PASSWORD = "MQTT_PASSWORD"; // password for mqtt clients. Set your own value here.

const char* ssid = "WIFI_SSID";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "WIFI_PASSWORD"; // The password of the Wi-Fi network

const char* MQTT_MSG = "Ding Dong! Someone is at the door.";
const char* MQTT_TOPIC = "home/doorbell";

const unsigned short mqttPort = 1883;
bool bellPressed = false;
bool isPlaying = false;  // Track if audio is playing
#define LED_PIN 2  // Define the GPIO pin for the LED (GPIO5 corresponds to D1 on ESP8266)

class MyBroker:public sMQTTBroker
{
public:
    bool onEvent(sMQTTEvent *event) override
    {
        switch(event->Type())
        {
        case NewClient_sMQTTEventType:
            {
                sMQTTNewClientEvent *e = (sMQTTNewClientEvent*)event;
		            // Check username and password used for new connection
                if ((e->Login() != MQTT_CLIENT_USER) || (e->Password() != MQTT_CLIENT_PASSWORD)) {
                  Serial.println("[-] ERROR: Invalid username or password");  
                  return false;
                }
                else {
                  Serial.print("[+] SUCCESS: Client connected: ");  
                  Serial.println(e->Client()->getClientId().c_str());
                }
            };
            break;
        case RemoveClient_sMQTTEventType:
            {
              sMQTTRemoveClientEvent *e = (sMQTTRemoveClientEvent*)event;
              Serial.print("[+] SUCCESS: Client disconnected: ");  
              Serial.println(e->Client()->getClientId().c_str());
            }
            break;
        case LostConnect_sMQTTEventType:
            {
              Serial.println("[!] WARN: WiFi connection lost. Retrying...");  
              WiFi.reconnect();
            }
            break;
        case UnSubscribe_sMQTTEventType:
        case Subscribe_sMQTTEventType:
            {
                sMQTTSubUnSubClientEvent *e = (sMQTTSubUnSubClientEvent*)event;
            }
            break;
        case Public_sMQTTEventType:
            {
              sMQTTPublicClientEvent *e = (sMQTTPublicClientEvent*)event;
              Serial.print("[#] MQTT: Received message '");
              Serial.print(e->Payload().c_str()); 
              Serial.print("' on topic '"); 
              Serial.print(e->Topic().c_str()); 
              Serial.print("' from Client : ");
              Serial.println(e->Client()->getClientId().c_str());
              // compare expected topic/ message combination
              if (e->Payload() == MQTT_MSG && e->Topic() == MQTT_TOPIC) {
                Serial.println("[*] INFO: Received doorbell message on home/doorbell"); 
                bellPressed = true;
              }
            }
            break;
        }
        return true;
    }
};

void setupWifi() {
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
        delay(1000);
    }
    WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
    Serial.println("[+] SUCCESS: Connected to Wi-Fi!");  
    Serial.print("[*] INFO: IP address:\t");
    Serial.println(WiFi.localIP());
}

void setupAudio() {
  // Initialize audio objects
  audioLogger = &Serial;
  file = new AudioFileSourcePROGMEM(doorbell, sizeof(doorbell));
  out = new AudioOutputI2SNoDAC();
  wav = new AudioGeneratorWAV();
  if (!file || !out || !wav) {
    Serial.println("[-] ERROR: Audio components not properly initialized.");
    return;
  }
  out->SetGain(1.0);
}

void handleAudioPlayback() {
  // Check if audio is playing
  if (wav->isRunning())
  {
    if (!wav->loop())
    {
      out->flush();
      wav->stop();
      out->stop();

      delete file;
      file = new AudioFileSourcePROGMEM(doorbell, sizeof(doorbell));

      isPlaying = false; // Mark playback as stopped
      Serial.println("[!] INFO: Audio playback finished.");
    }
  }
}

void playAudio() {
  
  if (!file->isOpen()) {
    Serial.println("[!] INFO: Reinitializing audio source...");
    delete file;
    file = new AudioFileSourcePROGMEM(doorbell, sizeof(doorbell));
  }
  
  if (!wav->begin(file, out)) {
    Serial.println("[-] ERROR: Failed to start audio playback.");
    return;
  }
  Serial.println("[!] INFO: Audio playback started.");
  isPlaying = true; // Mark as playing
}

MyBroker broker;
unsigned long Time;
unsigned long freeRam;

void setup()
{
    Serial.begin(9600);
    // Configure the LED pin as an OUTPUT
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // Turn off the LED initially
    setupWifi();
    setupAudio();
    
    broker.init(mqttPort);
    Serial.print("[+] SUCCESS: MQTT Broker started on IP: ");
    Serial.println(WiFi.localIP());
    Time=millis();
    freeRam=ESP.getFreeHeap();
};

unsigned long previousMillis = 0;   // Stores the last time the LED was updated
unsigned long interval = 100;        // Initial interval for LED blinking (100ms)
bool ledState = LOW;                 // Current LED state
int state = 0;                       // State for LED blinking pattern
bool isBlinking = false;

void blinkLED() {
  unsigned long currentMillis = millis();  // Get the current time
  if (currentMillis - previousMillis >= interval) {  // Check if interval has passed
    previousMillis = currentMillis;  // Save the last time LED was updated
    
    // Toggle the LED state
    digitalWrite(LED_PIN, ledState);
    ledState = !ledState;  // Toggle the LED state (LOW -> HIGH, or HIGH -> LOW)
    
    // Update the interval and state
    switch (state) {
      case 0:  
        interval = 100;
        state++;
        break;
      case 1:  
        interval = 200;
        state++;
        break;
      case 2: 
        interval = 1000;
        state++;
        break;
      case 3:  
        interval = 1000;
        state++;
        break;
      case 4:  
        interval = 1500;
        state++;
        break;
      case 5: 
        state = 0;  // Reset to the first state
        isBlinking = false;
        break;
    }
  }
}

void loop()
{
    if (bellPressed) {    // doorbell message received
      if(!isPlaying){     // check if audio is playing
        playAudio();      // play audio if not already playing
      }
      if(!isBlinking) {       // check if LED is not blinking
        //ledState = HIGH;
        isBlinking = true;    // turn on blinker flag -  to be picked up later
      }
      bellPressed = false;    // turn off bell pressed flag
    }
    
    if (isPlaying) {          // check if audio is playing
      handleAudioPlayback();  // handle audio playback & get ready for next playback loop
      if (isBlinking){        // check if blinker flag is true
        blinkLED();           // call LED blinking function in loop untill blinker flag is false
      }
    }

    broker.update();

    if(millis()-Time>300000)
    {
      Time=millis();
      if(ESP.getFreeHeap()!=freeRam)
      {
        freeRam=ESP.getFreeHeap();
        Serial.print("[*] LOOP_INFO: Free RAM:");
        Serial.println(freeRam);
      }
      Serial.print("[**] Broker running, listening for connections on IP: ");
      Serial.println(WiFi.localIP());
    }
    yield();
};