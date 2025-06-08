#include <ESP8266WiFi.h>
#include <WebSocketsServer.h> // by Markus Sattler
#include <ArduinoJson.h>      // For parsing JSON from webpage

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <DHT.h>              // For DHT sensor

/* Wi-Fi credentials */
const char* ssid = "Sankar_AC";     // Your AP SSID
const char* password = "12345678"; // Your AP Password

/* WebSocket server */
WebSocketsServer webSocket = WebSocketsServer(81);

/* IR Remote Configuration */
const uint16_t kIrLedPin = D5; // GPIO14 (NodeMCU D5)
IRsend irsend(kIrLedPin);

/* DHT Sensor Configuration */
#define DHTPIN D7       // GPIO13 (NodeMCU D7)
#define DHTTYPE DHT22   // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);
float currentRoomTemp = 0.0;
float currentRoomHumidity = 0.0;
unsigned long lastDhtReadTime = 0;
const unsigned long dhtReadInterval = 5000; // Read DHT every 5 seconds

/* Relay Configuration */
const int relay1Pin = D1; // GPIO5
const int relay2Pin = D2; // GPIO4
const int relay3Pin = D6; // GPIO12
bool relay1State = false;
bool relay2State = false;
bool relay3State = false;

const uint8_t VOLTAS_CMD_LENGTH = 10;
uint8_t Signal[VOLTAS_CMD_LENGTH];

// -------- FORWARD DECLARATIONS --------
void broadcastStatus();
void sendStatusToClient(uint8_t clientNum);
// --------------------------------------

// Function to generate the Voltas IR signal based on desired state
void prepareVoltasSignal(String power, String mode, int temp, String fanSpeed) {
  // 1. POWER OFF - This is always specific and overrides other settings
  if (power.equalsIgnoreCase("OFF")) {
    Signal[0] = 0x33; Signal[1] = 0x28; Signal[2] = 0x08; Signal[3] = 0x18;
    Signal[4] = 0x3B; Signal[5] = 0x3B; Signal[6] = 0x3B; Signal[7] = 0x11;
    Signal[8] = 0x20; Signal[9] = 0xA2;
    return;
  }

  // 2. POWER ON Logic
  if (power.equalsIgnoreCase("ON")) {
    // Common bytes that are mostly fixed for ON states initially
    Signal[0] = 0x33;
    Signal[4] = 0x3B; Signal[5] = 0x3B; Signal[6] = 0x3B;
    Signal[7] = 0x11; Signal[8] = 0x20;

    // Clamp temperature input
    int current_temp_val = temp;
    if (current_temp_val < 16) current_temp_val = 16;
    if (current_temp_val > 30) current_temp_val = 30;
    uint8_t current_temp_byte = (uint8_t)current_temp_val; 

    bool useExplicitFanIR = fanSpeed.equalsIgnoreCase("LOW") ||
                            fanSpeed.equalsIgnoreCase("MEDIUM") ||
                            fanSpeed.equalsIgnoreCase("HIGH") ||
                            fanSpeed.equalsIgnoreCase("AUTO");

    if (mode.equalsIgnoreCase("COOL")) {
      if (useExplicitFanIR) { 
        Signal[2] = 0x80; 
        Signal[3] = current_temp_byte;
        if (fanSpeed.equalsIgnoreCase("LOW"))       { Signal[1] = 0x88; Signal[9] = 0xE2 - current_temp_byte; }
        else if (fanSpeed.equalsIgnoreCase("MEDIUM")){ Signal[1] = 0x48; Signal[9] = 0x22 - current_temp_byte; }
        else if (fanSpeed.equalsIgnoreCase("HIGH")) { Signal[1] = 0x28; Signal[9] = 0x42 - current_temp_byte; }
        else if (fanSpeed.equalsIgnoreCase("AUTO")) { Signal[1] = 0xE8; Signal[9] = 0x82 - current_temp_byte; }
      } else { 
        Signal[1] = 0x28; Signal[2] = 0x88; Signal[3] = current_temp_byte; Signal[9] = 0x3A - current_temp_byte;
      }
    } else if (mode.equalsIgnoreCase("HEAT")) { 
      Signal[1] = 0x22; Signal[2] = 0x88; Signal[3] = current_temp_byte; Signal[9] = 0x40 - current_temp_byte;
    } else if (mode.equalsIgnoreCase("DRY")) { 
      Signal[1] = 0x84; Signal[2] = 0x88; Signal[3] = 0x18; Signal[9] = 0xC6;
    } else if (mode.equalsIgnoreCase("FAN")) { 
      Signal[1] = 0x41; Signal[2] = 0x88; Signal[3] = 0x10; Signal[9] = 0x11;
    } else { 
      Serial.print("Warning: Unknown MODE '"); Serial.print(mode); Serial.println("'. Defaulting to COOL (no explicit fan).");
      Signal[1] = 0x28; Signal[2] = 0x88; Signal[3] = current_temp_byte; Signal[9] = 0x3A - current_temp_byte;
    }
  } else { 
    Serial.print("Error: Invalid POWER status '"); Serial.print(power); Serial.println("'. Defaulting to POWER OFF.");
    prepareVoltasSignal("OFF", "", 0, ""); 
  }
}

// Function to send current sensor data and relay states to all clients
void broadcastStatus() {
  StaticJsonDocument<256> statusDoc;
  statusDoc["roomTemp"] = currentRoomTemp;
  statusDoc["humidity"] = currentRoomHumidity;
  
  JsonObject relay_states = statusDoc.createNestedObject("relay_states");
  relay_states["relay1"] = relay1State ? "ON" : "OFF";
  relay_states["relay2"] = relay2State ? "ON" : "OFF";
  relay_states["relay3"] = relay3State ? "ON" : "OFF";

  String output;
  serializeJson(statusDoc, output);
  webSocket.broadcastTXT(output);
}

// Function to send current sensor data and relay states to a specific client
void sendStatusToClient(uint8_t clientNum) {
  StaticJsonDocument<256> statusDoc;
  statusDoc["roomTemp"] = currentRoomTemp;
  statusDoc["humidity"] = currentRoomHumidity;
  
  JsonObject relay_states = statusDoc.createNestedObject("relay_states");
  relay_states["relay1"] = relay1State ? "ON" : "OFF";
  relay_states["relay2"] = relay2State ? "ON" : "OFF";
  relay_states["relay3"] = relay3State ? "ON" : "OFF";

  String output;
  serializeJson(statusDoc, output);
  webSocket.sendTXT(clientNum, output);
  Serial.println("Sent initial status to client " + String(clientNum) + ": " + output);
}


void setup() {
  Serial.begin(115200);
  Serial.println("\n\nVoltas AC IR Controller + DHT + Relays (Corrected Declarations)");

  pinMode(relay1Pin, OUTPUT); digitalWrite(relay1Pin, LOW);
  pinMode(relay2Pin, OUTPUT); digitalWrite(relay2Pin, LOW);
  pinMode(relay3Pin, OUTPUT); digitalWrite(relay3Pin, LOW);
  
  dht.begin();
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: "); Serial.println(WiFi.softAPIP());

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started.");

  irsend.begin();
  pinMode(kIrLedPin, OUTPUT); digitalWrite(kIrLedPin, LOW);
  Serial.println("IR Transmitter Initialized.");

  prepareVoltasSignal("OFF", "COOL", 24, "LOW"); 
  Serial.print("Initial IR Signal (AC OFF): {");
    for (int i = 0; i < VOLTAS_CMD_LENGTH; ++i) {
      Serial.printf("0x%02X", Signal[i]); if (i < VOLTAS_CMD_LENGTH - 1) Serial.print(", ");
    }
  Serial.println("}");
}

void loop() {
  webSocket.loop();

  unsigned long currentTime = millis();
  if (currentTime - lastDhtReadTime >= dhtReadInterval) {
    lastDhtReadTime = currentTime;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    bool dhtError = isnan(h) || isnan(t);
    if (dhtError) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      currentRoomTemp = t;
      currentRoomHumidity = h;
    }
    broadcastStatus(); 
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    String commandJson = String((char*)payload);
    Serial.println("JSON received from client " + String(num) + ": " + commandJson);

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, commandJson);

    if (error) {
      Serial.print("deserializeJson() failed: "); Serial.println(error.f_str());
      return;
    }

    const char* messageType = doc["type"];
    const char* action = doc["action"];

    if (messageType != nullptr && strcmp(messageType, "ac_control") == 0) {
      String reqPower = doc["power"];
      int reqTemp = doc["temp"];
      String reqMode = doc["mode"];
      String reqFan = doc["fan"] | "NONE"; 

      Serial.print("AC Command: P="); Serial.print(reqPower); Serial.print(", T="); Serial.print(reqTemp);
      Serial.print(", M="); Serial.print(reqMode); Serial.print(", F="); Serial.println(reqFan);

      prepareVoltasSignal(reqPower, reqMode, reqTemp, reqFan);

      Serial.print("Generated IR Signal: {");
      for (int i = 0; i < VOLTAS_CMD_LENGTH; ++i) {
        Serial.printf("0x%02X", Signal[i]);
        if (i < VOLTAS_CMD_LENGTH - 1) Serial.print(", ");
      }
      Serial.println("}");
      irsend.sendVoltas(Signal, VOLTAS_CMD_LENGTH);
      Serial.println("IR Signal Sent.");

    } else if (messageType != nullptr && strcmp(messageType, "relay_control") == 0) {
      String relayId = doc["relay"];
      String relayCmd = doc["state"];
      bool newState = relayCmd.equalsIgnoreCase("ON");

      Serial.print("Relay Command: ID="); Serial.print(relayId); Serial.print(", State="); Serial.println(relayCmd);

      if (relayId.equalsIgnoreCase("relay1")) {
        relay1State = newState;
        digitalWrite(relay1Pin, relay1State ? HIGH : LOW);
      } else if (relayId.equalsIgnoreCase("relay2")) {
        relay2State = newState;
        digitalWrite(relay2Pin, relay2State ? HIGH : LOW);
      } else if (relayId.equalsIgnoreCase("relay3")) {
        relay3State = newState;
        digitalWrite(relay3Pin, relay3State ? HIGH : LOW);
      }
      broadcastStatus();

    } else if (action != nullptr && strcmp(action, "get_initial_states") == 0) {
        Serial.println("Client " + String(num) + " requested initial states.");
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        if (!isnan(h) && !isnan(t)) {
          currentRoomTemp = t;
          currentRoomHumidity = h;
        }
        sendStatusToClient(num);
    } else {
      Serial.println("Unknown command type or action: " + commandJson);
    }
    delay(50);
  } else if (type == WStype_CONNECTED) {
    Serial.printf("[%u] Client connected: %s\n", num, webSocket.remoteIP(num).toString().c_str());
  } else if (type == WStype_DISCONNECTED) {
    Serial.printf("[%u] Client disconnected.\n", num);
  }
}
