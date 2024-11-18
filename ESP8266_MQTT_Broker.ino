#include <ESP8266WiFi.h>
#include "uMQTTBroker.h"

/*
 * Your WiFi config here
 */
const char ssid[] = "YOUR_SSID";     // your network SSID (name)
const char pass[] = "YOUR_PASS"; // your network password
/*
 * MQTT Authentication Configuration
 */
const char MQTT_USERNAME[] = "root";  // Set your own username here
const char MQTT_PASSWORD[] = "toor";  // Set your own password here

const char DATA_MSG[] = "Ding Dong! Someone is at the door.";
const String STR_TOPIC = "home/doorbell";
bool ringBell = false;

#define LED_PIN 2  // Define the GPIO pin for the LED (GPIO5 corresponds to D1 on ESP8266)

/*
 * Custom broker class with overwritten callback functions
 */
class myMQTTBroker: public uMQTTBroker
{
public:
    virtual bool onConnect(IPAddress addr, uint16_t client_count) {
      Serial.println("SUCCESS: "+addr.toString()+" connected");
      return true;
    }

    virtual void onDisconnect(IPAddress addr, String client_id) {
      Serial.println("INFO: "+addr.toString()+" ("+client_id+") disconnected");
    }

    virtual bool onAuth(String username, String password, String client_id) {
      Serial.println("INFO: Username/Password/ClientId: "+username+"/"+password+"/"+client_id);
      if (username == MQTT_USERNAME && password == MQTT_PASSWORD) {
          Serial.println("SUCCESS: Authentication successful!");
          return true;  // Allow connection
      } else {
          Serial.println("ERROR: Authentication failed!");
          return false;  // Deny connection
      }
    }
    
    virtual void onData(String topic, const char *data, uint32_t length) {
      char *data_str = new char[length + 1]; 
      memcpy(data_str, data, length);
      data_str[length] = '\0';
      
      
      Serial.print("INFO: Received data '");
      Serial.print(data_str);
      Serial.print("'");
      Serial.println(" on topic '"+topic+"'");
      if(strcmp(data_str,DATA_MSG) == 0 && topic.equals(STR_TOPIC)) {
        Serial.println("INFO: Doorbell pressed...");
        ringBell = true;
      }
      delete[] data_str;
    }

    // Sample for the usage of the client info methods
    virtual void printClients() {
      for (int i = 0; i < getClientCount(); i++) {
        IPAddress addr;
        String client_id;
         
        getClientAddr(i, addr);
        getClientId(i, client_id);
        Serial.println("INFO: Client "+client_id+" on addr: "+addr.toString());
      }
    }
};

myMQTTBroker myBroker;

void startWiFiClient()
{
  Serial.println("INFO: Connecting to "+(String)ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  Serial.println("SUCCESS: WiFi connected!");
}


void bellRinger() {
  Serial.println("SUCCESS: Doorbell pressed, play Doorbell Audio.");
  // tbd - more doorbell logic here
}

void setup()
{
  Serial.begin(9600);
  // Configure the LED pin as an OUTPUT
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // Turn off the LED initially

  startWiFiClient();
  // Start the broker
  Serial.println("INFO: Starting MQTT broker");
  myBroker.init();
  Serial.println("SUCCESS: MQTT broker started on IP: "+WiFi.localIP().toString());

/*
 * Subscribe to topic
 */
  myBroker.subscribe("test/topic");
}

unsigned long lastRestartTime = 0;
const unsigned long restartInterval = 1 * 60 * 1000;  // Restart every minute
void loop()
{
  if (ringBell) {
    bellRinger();
    ringBell = false;
  }
  /*
  if (millis() - lastRestartTime > restartInterval) {
    Serial.println("INFO: Restarting MQTT broker to free resources...");
    ESP.restart();
    Serial.println("SUCCESS: MQTT broker re-started on IP: "+WiFi.localIP().toString());
    lastRestartTime = millis();
  }
  */
}
