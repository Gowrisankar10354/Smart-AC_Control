# 🌬️❄️ IoT Smart AC & Environment Control Hub 🌡️💡

**Transform your standard Air Conditioner into a smart, web-controlled, and energy-efficient appliance!** This project brings modern IoT capabilities to your fingertips, allowing for intricate control over your room's climate and connected devices. The main aim is to control your AC and other appliances with a user-friendly digital application for a better, simpler life with enhanced functionality and energy awareness.

This project was inspired and guided by Professor Sujoy Saha – a big thank you for the insightful idea! 🙏

---

## 🌟 What This Project Does

This system leverages an ESP8266 microcontroller to:

1.  **Host a Web Server:** Creates a local Wi-Fi Access Point that serves a feature-rich webpage.
2.  **Provide Smart AC Control:** Enables you to control your AC's **Power**, **Mode** (Cool, Heat, Dry, Fan), **Temperature**, and **Fan Speed** (Low, Medium, High, Auto) remotely via the webpage.
3.  **Monitor Room Environment:** Integrates a DHT22 sensor to display **real-time temperature and humidity** on the web interface.
4.  **Control External Relays:** Manages up to **3 external relays** for other appliances like ceiling fans, lights, or even the main power supply to the AC system itself.
5.  **Communicate via WebSockets:** Ensures fast, bidirectional communication between your browser and the ESP8266 for instant response.

The result? A seamless and intelligent way to manage your comfort and energy consumption!

---

## ✨ Key Features & Special Functionalities

*   🌐 **WiFi Enabled & Web Interface:** No internet dependency for core control! The ESP8266 creates its own Wi-Fi network. Connect any device with a browser (phone, tablet, laptop).
*   🌡️ **Live Temperature & Humidity Data:** Stay informed about your room's environment directly on the control panel.
*   💨 **Precise Fan Speed Control:** Beyond simple ON/OFF, choose between Low, Medium, High, and Auto fan speeds.
*   💡 **Integrated Relay System:** Control other devices alongside your AC. Imagine turning off your room fan when the AC kicks in, all from one interface!
*   🧠 **Intelligent IR Signal Generation:** The ESP8266 doesn't just store fixed codes; it dynamically constructs complex 80-bit IR signals based on your selections for temperature, mode, and fan speed using arrays and arithmetic operations.
*   🛡️ **Connection Failsafe:** If your control device disconnects from the ESP8266 server, the AC automatically adjusts to a safe default (28°C, Low fan speed) instead of continuing a potentially extreme setting.
*   🔋 **Power Saving Auto-Off:** To prevent energy wastage, if no client device is connected to the ESP8266 for 20 minutes, the entire control system (including the ESP8266 itself) can automatically power down.
*   🛑💡 **Intelligent Zero-Standby Power System (A Game Changer!):**
    This unique system allows the *entire control unit* (ESP8266 and associated circuits) to be completely powered off, eliminating standby power consumption.
    *   **True System OFF:** One of the relays can be configured as a "System Relay." When this relay is turned OFF (either via the webpage or an optional physical master switch for the unit), it physically cuts power to the ESP8266 microcontroller and other relays. The ESP8266 shuts down, its WiFi network disappears, and there's zero standby power drain from the controller.
    *   **Smart System ON:** A dedicated external push-button switch is used for power-up. When you press and hold this button for a few seconds, it bypasses the (currently off) System Relay and provides initial power to the ESP8266. Once the ESP8266 boots up, one of its first software actions is to energize the System Relay, which then "latches" itself ON, keeping the ESP8266 and the rest of the system powered. The system now stays ON until commanded OFF again via the webpage's System Relay control or the physical master switch.
    This makes the system incredibly energy-efficient when not actively being used.

---

## 🤔 How It Works (The Basic Flow)

1.  **ESP8266 Setup:** The ESP8266 microcontroller boots up, creates a Wi-Fi Access Point (e.g., "Sankar_AC"), and starts a WebSocket server. It also initializes the DHT22 sensor and IR transmitter.
2.  **Connect to ESP8266:** You connect your phone or laptop to the ESP8266's Wi-Fi network.
3.  **Open Webpage:** You navigate to the ESP8266's IP address (usually `192.168.4.1`) in your browser. The smart control webpage loads.
4.  **WebSockets Magic:** The webpage establishes a WebSocket connection with the ESP8266 for real-time, two-way communication.
5.  **User Interaction:**
    *   You click buttons on the webpage (e.g., increase temperature, change mode, toggle a relay).
    *   The webpage sends a JSON command to the ESP8266 via WebSockets (e.g., `{"type":"ac_control", "power":"ON", "temp":24, "mode":"COOL", "fan":"AUTO"}`).
6.  **ESP8266 Processes:**
    *   Parses the JSON command.
    *   For AC commands, it dynamically generates the specific 10-byte IR code array.
    *   For relay commands, it toggles the corresponding GPIO pin.
    *   Sends the IR signal to your AC unit.
7.  **Feedback Loop:**
    *   The ESP8266 periodically reads data from the DHT22 sensor (temperature and humidity).
    *   It broadcasts this sensor data and the current state of the relays back to all connected webpage clients via WebSockets.
    *   The webpage updates the display with this live information.

This creates a responsive and interactive experience!

---

## 🛠️ Components Needed

*   ESP8266 Microcontroller (NodeMCU or similar)
*   IR Emitter LED
*   DHT22 (or DHT11) Temperature and Humidity Sensor
*   3 x 5V Relay Modules (or a multi-channel relay board)
*   Optional: Resistors for IR LED and DHT sensor pull-up (often included on modules)
*   Connecting Wires (Jumper wires)
*   5V Power Supply for ESP8266 & Relays
*   For the Intelligent ON/OFF system:
    *   One additional relay to act as the "System Relay" (can be one of the 3)
    *   A momentary push-button switch
    *   A main power switch for the whole unit (optional, for ultimate override)
*   Breadboard or PCB for assembly
*   An Air Conditioner unit that can be controlled by an IR remote.

---

## 🚀 How This Project Makes Life Better

*   **Simplicity:** Control your AC and other room appliances from a single, intuitive web interface. No more hunting for multiple remotes!
*   **Comfort:** Maintain your desired room temperature and humidity with ease. Get live feedback.
*   **Functionality:** Go beyond standard AC remote capabilities by integrating relay controls for fans, lights, etc.
*   **Energy Efficiency:**
    *   The "Intelligent Zero-Standby Power System" ensures no wasted energy when the controller is not needed.
    *   The "Power Saving Auto-Off" prevents the system from running indefinitely if forgotten.
    *   Better awareness of room conditions can lead to more optimized AC usage.
*   **Accessibility:** Control your environment from anywhere within the ESP8266's Wi-Fi range.
*   **Learning & Fun:** A fantastic project to learn about IoT, web servers on microcontrollers, WebSockets, IR communication, and sensor integration!

---

## 🔮 Future Scope & Possibilities

This project is a solid foundation! Here are some ideas for future enhancements:
*   🌐 **Internet Connectivity & Cloud Control:** Integrate with an MQTT broker or a cloud platform (like Blynk, Adafruit IO) for control from anywhere in the world.
*   🗣️ **Voice Control:** Add integration with voice assistants like Alexa or Google Assistant.
*   📅 **Scheduling & Timers:** Implement features to schedule AC operations.
*   📈 **Data Logging & Analytics:** Store sensor data to analyze trends and optimize energy usage further.
*   🤝 **ESP-NOW Mesh Network:** As mentioned, connect multiple such AC control units throughout a house using ESP-NOW. One ESP could act as a central gateway, and you could select which room's AC to control from a master web interface. This would make for a truly integrated smart home climate system.
*   📱 **Dedicated Mobile App:** While the webpage is responsive, a native mobile app could offer an even smoother experience and features like push notifications.
*   🤖 **Machine Learning:** Implement adaptive learning algorithms that adjust AC settings based on user habits, time of day, or even external weather data.

The possibilities are vast!

---


## 🔗 Project Resources

*   **Full Code & Schematics:** [Link to your GitHub Repository Subfolder for Code/Schematics] & ![circuit_image](https://github.com/user-attachments/assets/8a812a8d-a28b-4642-ac76-0d5d94868d42)

*   **Detailed Project Report/Paper:** [material1.pdf](https://github.com/user-attachments/files/20643032/DOC-20250502-WA0001.pdf)

*   **Presentation Slides:**
    *   [IR transmittion.pptx](https://www.canva.com/design/DAGnVhXs62Q/2NHqA2JfdP1UOHdX5hrqKg/view?utm_content=DAGnVhXs62Q&utm_campaign=designshare&utm_medium=link2&utm_source=uniquelinks&utlId=hf17434719f)
    *   [Smart AC_Control.pptx](https://www.canva.com/design/DAGnVhXs62Q/2NHqA2JfdP1UOHdX5hrqKg/view?utm_content=DAGnVhXs62Q&utm_campaign=designshare&utm_medium=link2&utm_source=uniquelinks&utlId=hf17434719f)

*   **Demo Videos:** 

[video](https://github.com/user-attachments/assets/e5e2d6fb-7ea1-4b2c-b6ab-66b0ab794171
)


---



