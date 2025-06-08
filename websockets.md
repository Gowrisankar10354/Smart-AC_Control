# 📱 Webpage Controller & WebSocket Interface Details

This document dives into the workings of the web-based interface used to control the Smart AC Hub. The frontend is built with HTML, CSS (TailwindCSS), and JavaScript, leveraging WebSockets for real-time communication with the ESP8266 server.

---

## 🌐 Understanding WebSockets

### What are WebSockets?
The WebSocket protocol (defined in RFC 6455) enables **two-way communication** between a client (your web browser) and a server (the ESP8266 in this case) over a single, long-lived TCP connection. Once the connection is established, both parties can send data to each other at any time, making it ideal for real-time applications.

### Why WebSockets for This Project?
*   **Bidirectional Communication:** Both the webpage needs to send commands (e.g., "Turn AC ON") and the ESP8266 needs to send updates (e.g., current temperature, relay states) without the client constantly having to ask (poll).
*   **Low Latency:** After the initial handshake, data frames are small, leading to quicker message delivery compared to repeated HTTP requests. This results in a responsive user interface.
*   **Efficiency:** Maintaining a single open connection is more efficient than establishing multiple HTTP connections for frequent updates, especially on resource-constrained devices like the ESP8266.

### WebSockets vs. HTTP
*   **HTTP (HyperText Transfer Protocol):** Primarily a request-response protocol. The client sends a request, and the server sends a response. For real-time updates with HTTP, techniques like polling or long-polling are needed, which can be inefficient.
*   **WebSockets:** Establishes a persistent, full-duplex connection. Once the "handshake" (which starts as an HTTP request with an `Upgrade` header) is successful, the protocol switches from HTTP to WebSockets, allowing free-flowing data in both directions.

---

## 🖥️ Webpage Functionality & Code Structure (Referencing your "Code1" Base)

The `index.html` file (from your Code1 version) contains the structure (HTML), styling (TailwindCSS via CDN and custom `<style>` block), and client-side logic (JavaScript in `<script>` tag).

### 1. Connecting to the ESP8266
   ```javascript
   let socket;
   const espIp = "192.168.4.1"; // ESP8266's SoftAP IP
   const espPort = 81;         // WebSocket port on ESP8266
   const wsUrl = `ws://${espIp}:${espPort}`;

   function connectWebSocket() {
       socket = new WebSocket(wsUrl);

       socket.onopen = () => {
           // Update UI to show "Connected"
           // Send an initial request to get current states from ESP
           socket.send(JSON.stringify({ action: "get_initial_states" }));
           // Optionally send current client-side AC state (if needed for sync)
           sendAcState();
       };

       socket.onclose = () => {
           // Update UI to show "Disconnected"
           // Attempt to reconnect after a delay
           setTimeout(connectWebSocket, 3000);
       };

       socket.onerror = (err) => {
           // Handle WebSocket errors
           console.error("Socket Error:", err);
           // (onclose will usually fire after an error too)
       };

       socket.onmessage = (event) => {
           // Handle messages received from the ESP8266
           handleIncomingMessage(event.data);
       };
   }
```
### 2. Sending Control Commands (AC & Relays)
   ```javascript
  //Command are sent as JSON string
  // Example: Sending AC Control State
  function sendAcState() {
      if (!socket || socket.readyState !== WebSocket.OPEN) return;
      const state = {
          type: "ac_control", // Message type for ESP8266 to identify
          power: isPowerOn ? "ON" : "OFF",
          temp: currentTemp,
          mode: acModes[currentModeIndex].name,
          fan: currentFanSpeed // Added fan speed
      };
      socket.send(JSON.stringify(state));
      console.log("Sent AC state:", state);
  }
  
  // Example: Sending Relay Control State
  function sendRelayState(relayId, isOn) {
      if (!socket || socket.readyState !== WebSocket.OPEN) return;
      const cmd = {
          type: "relay_control", // Message type
          relay: relayId,        // e.g., "relay1", "relay2"
          state: isOn ? "ON" : "OFF"
      };
      socket.send(JSON.stringify(cmd));
      console.log("Sent Relay state:", cmd);
  }
  ```
### 3. Receiving and Displaying Data
   ```javascript
    function handleIncomingMessage(jsonData) {
    try {
        const data = JSON.parse(jsonData);
        console.log("ESP says:", data);

        // Update Environment Data (Temperature & Humidity)
        if (data.roomTemp !== undefined) roomTemperature = parseFloat(data.roomTemp).toFixed(1);
        if (data.humidity !== undefined) roomHumidity = parseFloat(data.humidity).toFixed(1);
        
        // Update Relay States (expects a nested object or individual updates)
        if (data.relay_states) { // For initial bulk update
             Object.keys(data.relay_states).forEach(key => {
                  if (relayStates.hasOwnProperty(key)) {
                     relayStates[key] = (data.relay_states[key] === "ON");
                  }
             });
        }
        // Example: Handling a single relay update confirmation from ESP (optional)
        if (data.type === "relay_update" && data.relay && data.state !== undefined) {
            if (relayStates.hasOwnProperty(data.relay)) {
                 relayStates[data.relay] = (data.state === "ON");
            }
        }
        
        updateAllUIs(); // Refresh the entire UI with new states
    } catch (error) {
        console.error("Error parsing message from ESP:", error, "Data:", jsonData);
    }
  }
   ```
### 4. 🖼️ UI Elements and Responsiveness (Based on "Code1" HTML/JS)

The user interface is crafted with HTML, styled using TailwindCSS for a modern look and responsive design, and brought to life with client-side JavaScript. It aims to be intuitive on both desktop and mobile devices.

**Key UI Components:**

*   💨 **AC Controls:**
    *   **Power Button:** Toggles the AC ON/OFF. Visual state changes (e.g., color) indicate current status.
    *   **Temperature Controls:** Dedicated "+" and "-" buttons for increasing/decreasing the set temperature.
    *   **Temperature Display:** Clearly shows the current target temperature (e.g., "24°C").
    *   **Mode Button:** Cycles through available AC modes (COOL, DRY, HEAT, FAN).
    *   **Mode Icon:** Visually represents the currently selected AC mode (e.g., snowflake for COOL, sun for HEAT). Animates when the AC is ON.

*   🍃 **Fan Speed Controls:**
    *   **Visual Bar Display:**
        *   _Desktop:_ A vertical bar where levels (LOW, MEDIUM, HIGH, AUTO) fill upwards from bottom (LOW) to top (AUTO).
        *   _Mobile:_ A horizontal bar where levels fill from left (LOW) to right (AUTO).
    *   **Cycle Buttons:** Up/Down (or Left/Right on mobile) arrows to cycle through fan speeds: LOW ↔ MEDIUM ↔ HIGH ↔ AUTO.
    *   **Direct Level Selection:** Users can click directly on a speed level in the bar to select it.
    *   Active and filled states are visually highlighted.

*   🔌 **Relay Toggles:**
    *   Individual toggle switches for each of the 3 system relays (e.g., "Main Light," "Room Fan," "Aux Outlet").
    *   The visual state of the toggle reflects whether the relay is ON or OFF.

*   🌡️💧 **Live Environment Display:**
    *   Dedicated sections to show real-time **Room Temperature** and **Humidity** as received from the ESP8266 (via DHT22 sensor).

*   📱 **Mobile Navigation & Layout:**
    *   **Slide-Out Menu (`#mobileMenuDrawer`):** For smaller screens (typically <768px width), a slide-out navigation drawer provides access to different control sections/pages ("AC & Fan," "Relays," "Environment").
        *   **Trigger:** Activated by a "hamburger" icon (`dom.mobileMenuButton`).
        *   **Closing:** Can be closed by an "X" icon within the drawer (`dom.closeMenuButton`) or by clicking the semi-transparent overlay (`dom.menuOverlay`) that appears behind it.
    *   **Page Switching Logic (JavaScript):**
        *   The `dom.mobileMenuButton`, `dom.closeMenuButton`, and `dom.menuOverlay` elements have event listeners that toggle the `.open` class on the `#mobileMenuDrawer` element (controlling its visibility via CSS transforms) and the `.hidden` class on the `#menuOverlay`.
        *   Navigation links within the drawer (`dom.mobileNavLinks`, which are `<a>` tags with `data-page` and `data-title` attributes) have event listeners. When clicked, they call the `showMobilePage(pageId, pageTitle)` JavaScript function.
        *   `showMobilePage()` hides all mobile page containers and then displays the one matching the `pageId`, updates the mobile header title (`dom.mobilePageTitle`), and automatically closes the slide-out menu.
    *   **Dedicated Mobile Page Sections:** Content for "AC & Fan Controls," "System Relays," and "Room Statistics" are organized into separate containers that are shown/hidden based on menu selection.

*   📶 **Connection Status:** A visual indicator (dot + text, e.g., "🟢 Connected" or "🔴 Disconnected") at the top of the page keeps the user informed about the WebSocket connection status to the ESP8266.

The combination of TailwindCSS's utility classes and targeted JavaScript ensures that the UI adapts gracefully to different screen sizes and provides a seamless control experience.
### 3. Receiving and Displaying Data
   ```javascript
function updateAllUIs() {
    updateAcSection(dom.ac); updateAcSection(dom.acMobile);
    updateFanSpeedUIDesktop(); updateFanSpeedUIMobile();
    updateRelaysSection(dom.relaysDesktop, dom.relaysMobile);
    updateEnvironmentSection(dom.envDesktop, dom.envMobile);
    updateConnectionStatusUI(socket && socket.readyState === WebSocket.OPEN);
}
```
