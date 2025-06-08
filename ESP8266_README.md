# ⚙️ ESP8266 Smart Controller Firmware Details

This document provides an in-depth explanation of the C++/Arduino code running on the ESP8266, which forms the backend server and control logic for the Smart AC Hub.

---

## 🚀 Core Functionalities

The ESP8266 firmware is responsible for:

1.  🌐 **Wi-Fi Access Point:** Creates a local Wi-Fi network (e.g., "Sankar_AC") allowing devices to connect directly without needing an external router.
2.  🔗 **WebSocket Server:** Establishes and manages WebSocket connections with clients (web browsers) for real-time, bidirectional communication.
3.  ♨️ **IR Signal Generation & Transmission:** Intelligently constructs and sends precise 10-byte Infrared (IR) signals to control the Air Conditioner.
4.  🌡️ **DHT22 Sensor Integration:** Reads temperature and humidity data from an attached DHT22 sensor.
5.  🔌 **Relay Control:** Manages the state (ON/OFF) of up to 3 external relays connected to GPIO pins.
6.  🛡️ **Failsafe & Power Saving:** Implements logic for default AC settings on client disconnect and system auto-shutdown after prolonged inactivity.

---

## 🧠 Code Deep Dive: Intelligent IR Signal Generation

Controlling an Air Conditioner via IR is not just about sending a simple "ON" or "OFF" pulse. Most AC units expect a full state packet with every command, encoding power, mode, temperature, fan speed, and sometimes swing or other features, along with a checksum. For your Voltas AC, this is an 80-bit (10-byte) signal.

### Why Arrays for IR Signals?
*   **Data Size:** A standard 80-bit IR signal is too large to be stored in a single primitive variable type in C/C++ (e.g., `long long` is typically 64-bit).
*   **Byte-Level Manipulation:** AC IR protocols require precise control over individual bytes within the signal packet. Each byte often represents a specific setting or part of a setting.
*   **Structure:** An array (e.g., `uint8_t Signal[10];`) perfectly represents the sequence of 10 bytes that form the complete IR command. Each element `Signal[0]` through `Signal[9]` can be set individually.

### Decoding AC Protocols (General Approach)
To create this "intelligent" system, we first needed to understand how the AC's IR codes are structured. This often involves:
1.  Capturing raw IR codes from the original remote for various settings (different temperatures, modes, fan speeds).
2.  Analyzing these codes byte-by-byte to identify which bytes change for which settings.
3.  Reverse-engineering the arithmetic or logic used to calculate variable bytes (like checksums or temperature-dependent bytes).

*(This section alludes to the effort of providing the IR code tables.)*

### Dynamic Signal Construction in `prepareVoltasSignal()`
The `prepareVoltasSignal(String power, String mode, int temp, String fanSpeed)` function is the heart of the IR control logic. It dynamically builds the `Signal[10]` array:

1.  **Power OFF:** This is straightforward. A specific, fixed 10-byte array is loaded for "OFF".
2.  **Power ON:** This is more complex:
    *   **Base Bytes:** Some bytes are common to most "ON" states (e.g., `Signal[0]=0x33`, `Signal[4-8]`).
    *   **Temperature (`Signal[3]`):** The raw temperature value (16-30) is directly cast to a `uint8_t` and placed in `Signal[3]`.
    *   **Mode & Fan Prioritization:**
        *   **Explicit Fan Speeds (LOW, MED, HIGH, AUTO) in COOL mode:** When the user selects these fan speeds from the UI (and the mode is COOL), `Signal[2]` is set to `0x80`. `Signal[1]` takes a specific value for each fan speed (`0x88` for LOW, `0x48` for MED, etc.). Crucially, `Signal[9]` is then calculated using arithmetic based on *both* the fan speed and the current temperature:
            *   LOW Fan: `Signal[9] = 0xE2 - temperature_byte;`
            *   MEDIUM Fan: `Signal[9] = 0x22 - temperature_byte;`
            *   HIGH Fan: `Signal[9] = 0x42 - temperature_byte;`
            *   AUTO Fan: `Signal[9] = 0x82 - temperature_byte;`
        *   **Standard COOL Mode (No explicit L/M/H/A fan from UI):** If the mode is COOL but no specific fan speed (L/M/H/A) is actively chosen by the user from the fan speed controls, the system reverts to a "general COOL" IR structure. Here, `Signal[2]` becomes `0x88`, `Signal[1]` is fixed (e.g., `0x28`), and `Signal[9]` is calculated based on temperature (e.g., `0x3A - temperature_byte`). This allows simple temperature changes in COOL mode without always needing to specify a fan.
        *   **HEAT (Sun) Mode:** `Signal[2]` is `0x88`, `Signal[1]` is specific for HEAT (e.g., `0x22`), and `Signal[9]` is calculated via `0x40 - temperature_byte`. *Explicit fan speed settings from the UI currently do not override this specific HEAT structure unless further IR codes for "HEAT at specific fan" are decoded and added.*
        *   **DRY (Water) & FAN (Fan-Only) Modes:** These modes typically have fixed byte structures for `Signal[1]`, `Signal[2]`, `Signal[3]`, and `Signal[9]`, as the temperature and fan are often not user-adjustable in the same granular way.
    *   This layered logic ensures the most appropriate IR code is generated based on the combination of user inputs.
    *   for some of the raw signals can be checked here-> [signal data.txt](https://github.com/user-attachments/files/20643131/signal.data.txt)
    *   for detail and versatile signal understanding one can run this c program -> 

Once the `Signal` array is prepared, `irsend.sendVoltas(Signal, VOLTAS_CMD_LENGTH);` transmits it.

---

## 🔌 Hardware Components Integration

*   **ESP8266 (NodeMCU):** The microcontroller serving as the brain, Wi-Fi AP, and WebSocket server. (GPIOs D1, D2, D5, D6, D7 are used in the example code).
*   **IR Emitter LED (controlled by `kIrLedPin = D5`):** Connected to a GPIO pin (usually with a current-limiting resistor or driven by a transistor for better range if needed). The `IRsend` object from the `IRremoteESP8266` library handles the precise timing for IR transmission.
*   **DHT22 Temperature & Humidity Sensor (on `DHTPIN = D7`):** A digital sensor that provides temperature and humidity readings. The `DHT` library simplifies communication with it. A pull-up resistor (4.7kΩ-10kΩ) between VCC and Data pin is usually required if not on a breakout board.
*   **Relays (on `relay1Pin=D1`, `relay2Pin=D2`, `relay3Pin=D6`):** Standard 5V relay modules. The ESP8266 controls these by setting their respective GPIO pins HIGH or LOW. Remember that relays draw current, so ensure your power supply can handle the ESP8266 and active relays. For the **Intelligent ON/OFF System**, one of these relays (e.g., Relay3 if used as "System Relay") is wired to control the main power to the ESP8266 itself, with a momentary push-button switch to initiate power-up.

---

## 📚 Software Libraries Used

This project relies on several key Arduino libraries:

*   **`ESP8266WiFi.h`:** For Wi-Fi functionalities (Access Point mode).
    *   *Learn more:* [Link to ESP8266 WiFi Library Documentation - User to add:e.g., https://github.com/ekstrand/ESP8266]
*   **`WebSocketsServer.h`** (by Markus Sattler): For creating the WebSocket server on the ESP8266.
    *   *Learn more:* [Link to WebSocketsServer Library on GitHub - User to add: e.g., https://github.com/Links2004/arduinoWebSockets]
*   **`ArduinoJson.h`** (by Benoît Blanchon): For efficient parsing and generation of JSON data exchanged over WebSockets.
    *   *Learn more:* [Link to ArduinoJson Website/Docs - User to add: e.g., https://arduinojson.org/]
*   **`IRremoteESP8266.h`** (by David Conran et al.): A powerful library for sending (and receiving) Infrared signals with ESP8266/ESP32. We specifically use its `IRsend` capabilities and `sendVoltas()` method.
    *   *Learn more:* [Link to IRremoteESP8266 Library on GitHub - User to add: e.g., https://github.com/crankyoldgit/IRremoteESP8266]
*   **`DHT.h`** (by Adafruit or similar): For interfacing with DHT series temperature and humidity sensors.
    *   *Learn more:* [Link to Adafruit DHT Library on GitHub - User to add: e.g., https://github.com/adafruit/DHT-sensor-library]

---

## 💡 Failsafe & Power Saving Features

*   **Client Disconnect (`webSocketEvent` - `WStype_DISCONNECTED`):** While the current code doesn't explicitly set the AC to 28°C/Low fan on *every* disconnect event in `webSocketEvent` (as that might be too aggressive if there are brief network hiccups and multiple clients), the webpage itself will remember its last state. The *concept* of a failsafe if the controlling client *disappears for good* is a good design principle often handled by timeouts or logic in the client. *The previously mentioned default to 28°C if a device disconnects implies the UI on other devices might try to maintain control, or a future version would need more server-side disconnect logic if desired.*
*   **Auto System Power-Off (Not explicitly in the provided ESP code but described by user):** This crucial feature (shutting down after 20 minutes of no client connections) would require an additional timer in the ESP8266's `loop()`. This timer would reset every time a WebSocket client connects or sends a message. If the timer expires, the ESP8266 would then command the "System Relay" (the one controlling its own power) to turn OFF.
    ```cpp
    // Example Snippet for Auto Power-Off (to be integrated in loop() and webSocketEvent)
    // unsigned long lastClientActivityTime = 0;
    // const unsigned long systemShutdownTimeout = 20 * 60 * 1000; // 20 minutes
    // bool clientConnected = false; // Managed in onConnect/onDisconnect

    // In loop():
    // if (clientConnected && (millis() - lastClientActivityTime > systemShutdownTimeout)) {
    //     Serial.println("No client activity for 20 minutes. Shutting down system.");
    //     // Code to turn OFF the main system power relay
    //     // e.g., if relay3Pin controls main power:
    //     // relay3State = false; // Or whatever state means OFF for that specific relay
    //     // digitalWrite(relay3Pin, relay3State ? HIGH : LOW);
    //     // Then ESP8266 itself will lose power.
    // }

    // In webSocketEvent WStype_CONNECTED or WStype_TEXT from an active client:
    // lastClientActivityTime = millis();
    // clientConnected = true; // At least one client is active

    // In webSocketEvent WStype_DISCONNECTED:
    // if (webSocket.connectedClients() == 0) clientConnected = false; // No more clients
    ```

---

## ⚙️ Compilation and Uploading

1.  Ensure you have the ESP8266 board support package installed in your Arduino IDE.
2.  Install all the libraries listed above via the Arduino Library Manager.
3.  Select your ESP8266 board type (e.g., "NodeMCU 1.0 (ESP-12E Module)") and the correct COM port.
4.  Compile and upload the sketch.
5.  Monitor the Serial Output (Baud Rate 115200) for IP address and status messages.

---

This firmware turns the ESP8266 into a capable and intelligent hub for your AC and room environment control!
