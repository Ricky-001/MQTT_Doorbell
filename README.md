<h1>MQTT DOORBELL USING ESP32 AND ESP8266</h1>

This project uses an ESP32 and an ESP8266 NodeMCU to communicate via MQTT within a local network. <br/>
-
* Out of the two boards, the ESP32 acts as the doorbell switch and the ESP8266 acts as the MQTT broker as well as the receiver unit for the doorbell.
  * The ESP32 provides visual feedback by blinking an LED on the press of the doorbell button (connected to the board) and sends out the MQTT message, notifying the bell press.
  * The ESP8266 runs the MQTT broker to circulate the MQTT message to subscribed clients, while also acting as the receiver unit for the doorbell, providing audio-visual feedback.
* All clients connected to the ESP8266 MQTT Broker will receive the message and alerts can be set up accordingly.
* Please ensure to modify the credentials on the respective codes

---

<h2>DOORBELL RECEIVER</h2>

[doorbell_receiver_mqttbroker [ESP8266]](MQTT_Doorbell/doorbell_receiver_mqttbroker/doorbell_receiver_mqttbroker.ino)

* **Modify the SSID and Password**
https://github.com/Ricky-001/MQTT_Doorbell/blob/5ee4b4eb8552d3b1abdc83b6b3c202e939a1ce0d/MQTT_Doorbell/doorbell_receiver_mqttbroker/doorbell_receiver_mqttbroker.ino#L33-L34
of your home Wi-Fi (or the Wi-Fi you want the connected devices on)
* **Modify the MQTT UserID and MQTT Password**
https://github.com/Ricky-001/MQTT_Doorbell/blob/5ee4b4eb8552d3b1abdc83b6b3c202e939a1ce0d/MQTT_Doorbell/doorbell_receiver_mqttbroker/doorbell_receiver_mqttbroker.ino#L30-L31 
to the credentials you would like to use *(all clients need to use these credentials to connect and communicate/ listen to messages)*
  
* This code uses the [sMQTTBroker library](https://github.com/terrorsl/sMQTTBroker) to convert an ESP8266 NodeMCU into an MQTT broker on a local network, connected by Wi-Fi to enable MQTT communications over the local network.

* The ESP8266 will act as the MQTT broker, while also processing the incoming message for the doorbell press
* Thus it acts as the doorbell's receiver unit, providing audio-visual feedback. 

* The audio is played using the [ESP8266Audio libraries](https://github.com/earlephilhower/ESP8266Audio).
* The audio data is directly fed into the *doorbell.h* header file as a hex code.
* The audio file should be in WAV format, 16-bit PCM before converting the contents into Hex and pasting the contents into [doorbell.h](MQTT_Doorbell/doorbell_receiver_mqttbroker/doorbell.h)

<br/>

<h2>DOORBELL SWITCH</h2>

[doorbell_with_deep_sleep [ESP32]](MQTT_Doorbell/doorbell_with_deep_sleep/doorbell_with_deep_sleep.ino)

* This code uses the [PubSubClient library](https://github.com/knolleary/pubsubclient) to convert an ESP32 into an MQTT client
  that sends specific MQTT messages over the local network on the press of a button, which acts as the doorbell.

* The ESP32 remains in deep sleep while not in use but comes online, connects to Wi-Fi & MQTT and sends the MQTT message to a broker on the local network using pre-defined credentials
* **Modify the SSID and Password**
https://github.com/Ricky-001/MQTT_Doorbell/blob/5ee4b4eb8552d3b1abdc83b6b3c202e939a1ce0d/MQTT_Doorbell/doorbell_with_deep_sleep/doorbell_with_deep_sleep.ino#L19-L20
 of your home Wi-Fi (or the Wi-Fi you want the connected devices on)
* **Modify the MQTT UserID and MQTT Password** 
https://github.com/Ricky-001/MQTT_Doorbell/blob/5ee4b4eb8552d3b1abdc83b6b3c202e939a1ce0d/MQTT_Doorbell/doorbell_with_deep_sleep/doorbell_with_deep_sleep.ino#L19-L20
to whatever is set on the Broker code
* **Modify the IP address to that of your MQTT Broker on the local network**
https://github.com/Ricky-001/MQTT_Doorbell/blob/5ee4b4eb8552d3b1abdc83b6b3c202e939a1ce0d/MQTT_Doorbell/doorbell_with_deep_sleep/doorbell_with_deep_sleep.ino#L22

<br/>

---

## **Circuit Diagrams**

### **1. ESP32 (Doorbell Switch)**
The circuit includes an LED and a button. The ESP32 enters deep sleep when idle and wakes up when the button is pressed.

![ESP32 Diagram](https://github.com/user-attachments/assets/d700a357-a251-4ad2-bfed-b9af4b115e40)

### **2. ESP8266 (Receiver and Speaker)**
The circuit uses a transistor to amplify audio output for a speaker.

![ESP8266 Diagram](https://github.com/user-attachments/assets/c489205c-e768-4b5f-84ac-7e08a59dfc4d)

---

## **How to Download and Set Up This Project**

### **Step 1: Clone or Download the Project**
1. Clone this repository to your local machine using Git:
   ```bash
   git clone https://github.com/Ricky-001/MQTT_Doorbell.git
   ```
   Alternatively, you can click on the green **Code** button on the GitHub page and select **Download ZIP**. Then, extract the ZIP file to your computer.

### **Step 2: Open the Project in Arduino IDE**
1. Install the [Arduino IDE](https://www.arduino.cc/en/software) if you don’t already have it.
2. Open the `.ino` files for the ESP32 and ESP8266 codes:
   - `doorbell_with_deep_sleep.ino` for the ESP32 (doorbell switch).
   - `doorbell_receiver_mqttbroker.ino` for the ESP8266 (receiver and broker).

---

## **Required Libraries**

Before uploading the code to your boards, ensure the required libraries are installed. You can install these libraries via the Arduino Library Manager:
1. Open Arduino IDE and navigate to `Sketch > Include Library > Manage Libraries`.
2. Search for and install the following libraries:
   - **For ESP32 (doorbell switch):**
     - [PubSubClient](https://github.com/knolleary/pubsubclient): Used for MQTT communication.
     - WiFi: Pre-installed with the ESP32 board package.
   - **For ESP8266 (MQTT Broker and receiver):**
     - [sMQTTBroker](https://github.com/terrorsl/sMQTTBroker): Used to set up the MQTT broker.
     - [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio): Used for audio playback.
     - ESP8266WiFi: Pre-installed with the ESP8266 board package.

---

## **Step 3: Install ESP32 and ESP8266 Board Packages**

1. **Add ESP32 and ESP8266 Board URLs**:
   - Go to `File > Preferences`.
   - Add the following URLs under "Additional Boards Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
2. **Install Board Packages**:
   - Go to `Tools > Board > Boards Manager`.
   - Search for `ESP32` and `ESP8266`, then click **Install**.

---

## **Uploading Sketches to ESP32 and ESP8266**

### **Step 1: Connect the Board to Your Computer**
- Use a micro-USB cable to connect your ESP32 or ESP8266 to your computer. Ensure the cable supports data transfer (not just charging).

### **Step 2: Select the Correct Board and Port**
1. In the Arduino IDE, navigate to `Tools > Board` and select:
   - **ESP32 Dev Module** for the ESP32.
   - **NodeMCU 1.0 (ESP-12E Module)** for the ESP8266.
2. Under `Tools > Port`, select the port to which your board is connected.

### **Step 3: Configure and Upload the Sketch**
1. Open the `.ino` file for the respective board (ESP32 or ESP8266).
2. Modify the following in the code:
   - **Wi-Fi credentials** (SSID and password).
   - **MQTT Broker IP, username, and password** (ensure they match across both devices).
3. Click the **Upload** button in the Arduino IDE to flash the code to the board.
4. Monitor the Serial Monitor (set to 115200 baud rate) for debugging and ensuring the board connects successfully to Wi-Fi and the MQTT broker.

---

## **Troubleshooting Tips**

1. **Wi-Fi Connection Issues:**
   - Ensure your Wi-Fi SSID and password are entered correctly in both sketches.
   - Check if the ESP32 and ESP8266 are within range of the Wi-Fi router.
   - Ensure the Wi-Fi network operates on 2.4 GHz (not 5 GHz).

2. **MQTT Communication Issues:**
   - Verify that the ESP8266 is running the MQTT broker successfully by subscribing to its IP using a client app (e.g., MQTT Explorer or MQTT.fx).
   - Double-check the MQTT credentials in both the broker and the client code.

3. **Audio Playback Issues on ESP8266:**
   - Ensure your speaker wiring matches the schematic.
   - Verify that the audio file in `doorbell.h` is correctly converted to hex and is in the right format (16-bit PCM WAV).

---

## **Testing the Setup**

1. **Power Up the ESP8266:**
   - Once flashed, connect the ESP8266 to a 5V power source.
   - It should start running the MQTT broker.

2. **Power Up the ESP32:**
   - Press the button on the ESP32. The LED should blink, and an MQTT message should be sent.

3. **Verify MQTT Message Reception:**
   - Use a mobile app like [MQTT Alert](https://play.google.com/store/apps/details?id=gigiosoft.MQTTAlert) or [MQTT Explorer](https://mqtt-explorer.com/) to subscribe to the MQTT broker on the ESP8266's IP address.
   - Check if the doorbell message appears under the topic `home/doorbell`.

---

## **Setting Up Mobile Alerts**

Apps like **MQTT Alert** can notify you on your smartphone when the doorbell is pressed:
1. Install the app from the Play Store.
2. Add a new connection with the following settings:
   - **Broker IP**: IP address of the ESP8266.
   - **Port**: 1883.
   - **Username/Password**: As set in your ESP8266 MQTT Broker code.
   - **Topic**: `home/doorbell`.
3. Set up a notification tone to play when the message is received on the above topic.
4. You should be good to go. You'll receive mobile notifications each time your doorbell is pressed!
