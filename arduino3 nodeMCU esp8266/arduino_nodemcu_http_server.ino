// Include necessary libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi credentials
const char* ssid = "Mimi";
const char* password = "mimimimi1";

// Create an instance of the web server
ESP8266WebServer server(80);

// Variables to store channel statuses
bool channel1Status = false;
bool channel2Status = false;

// Define relay pins
const int relay1Pin = D0;
const int relay2Pin = D1;

// Ensure LED_BUILTIN is defined for NodeMCU
#define LED_BUILTIN 2  // GPIO2 (D4) for NodeMCU

// Define pins for the MH-Sensor
const int mhSensorDigitalPin = D2; // Digital output pin
const int mhSensorAnalogPin = A0;  // Analog output pin

// Function to handle root endpoint
void handleRoot() {
  String message = "<html><body><h1>NodeMCU Status</h1>";
  message += "<p>Channel 1: ";
  message += (channel1Status ? "ON" : "OFF");
  message += "</p><p>Channel 2: ";
  message += (channel2Status ? "ON" : "OFF");
  message += "</p></body></html>";
  server.send(200, "text/html", message);
}

// Function to toggle channel 1
void toggleChannel1() {
  channel1Status = !channel1Status;
  digitalWrite(relay1Pin, channel1Status ? HIGH : LOW);
  server.send(200, "text/plain", channel1Status ? "Channel 1 ON" : "Channel 1 OFF");
}

// Function to toggle channel 2
void toggleChannel2() {
  channel2Status = !channel2Status;
  digitalWrite(relay2Pin, channel2Status ? HIGH : LOW);
  server.send(200, "text/plain", channel2Status ? "Channel 2 ON" : "Channel 2 OFF");
}

// Function to handle status endpoint
void handleStatus() {
  String response = "{";
  response += "\"relay1\":\"" + String(channel1Status ? "ON" : "OFF") + "\",";
  response += "\"relay2\":\"" + String(channel2Status ? "ON" : "OFF") + "\",";
  response += "\"builtin_led\":\"" + String(digitalRead(LED_BUILTIN) == LOW ? "ON" : "OFF") + "\"";
  response += "}";
  server.send(200, "application/json", response);
}

// Function to turn built-in LED ON
void turnBuiltinLedOn() {
  Serial.println("Turning Built-in LED ON");
  digitalWrite(LED_BUILTIN, LOW); // Active LOW
  server.send(200, "application/json", "{\"status\":\"Built-in LED turned ON\"}");
}

// Function to turn built-in LED OFF
void turnBuiltinLedOff() {
  Serial.println("Turning Built-in LED OFF");
  digitalWrite(LED_BUILTIN, HIGH);
  server.send(200, "application/json", "{\"status\":\"Built-in LED turned OFF\"}");
}

// Function to toggle built-in LED
void toggleBuiltinLed() {
  bool isOn = (digitalRead(LED_BUILTIN) == LOW);
  digitalWrite(LED_BUILTIN, isOn ? HIGH : LOW);
  String state = isOn ? "OFF" : "ON";
  String response = "{\"status\":\"Built-in LED toggled to " + state + "\"}";
  server.send(200, "application/json", response);
}

// Function to turn Relay 1 (D0) ON
void turnRelay1On() {
  digitalWrite(relay1Pin, HIGH);
  server.send(200, "application/json", "{\"status\":\"Relay 1 turned ON\"}");
}

// Function to turn Relay 1 (D0) OFF
void turnRelay1Off() {
  digitalWrite(relay1Pin, LOW);
  server.send(200, "application/json", "{\"status\":\"Relay 1 turned OFF\"}");
}

// Function to turn Relay 2 (D1) ON
void turnRelay2On() {
  digitalWrite(relay2Pin, HIGH);
  server.send(200, "application/json", "{\"status\":\"Relay 2 turned ON\"}");
}

// Function to turn Relay 2 (D1) OFF
void turnRelay2Off() {
  digitalWrite(relay2Pin, LOW);
  server.send(200, "application/json", "{\"status\":\"Relay 2 turned OFF\"}");
}

// Function to read the digital output of the MH-Sensor
void readMHSensorDigital() {
  int digitalValue = digitalRead(mhSensorDigitalPin);
  String response = "{\"digital\":\"" + String(digitalValue == HIGH ? "HIGH" : "LOW") + "\"}";
  server.send(200, "application/json", response);
}

// Function to read the analog output of the MH-Sensor
void readMHSensorAnalog() {
  int analogValue = analogRead(mhSensorAnalogPin);
  String response = "{\"analog\":" + String(analogValue) + "}";
  server.send(200, "application/json", response);
}

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected!");

  // Print the IP address
  Serial.println(WiFi.localIP());

  // Initialize relay pins
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  digitalWrite(relay1Pin, LOW);
  digitalWrite(relay2Pin, LOW);

  // Initialize built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Ensure the LED is OFF initially

  // Initialize MH-Sensor pins
  pinMode(mhSensorDigitalPin, INPUT);
  pinMode(mhSensorAnalogPin, INPUT);

  // Define server routes
  server.on("/", handleRoot);
  server.on("/toggleChannel1", toggleChannel1);
  server.on("/toggleChannel2", toggleChannel2);
  server.on("/status", handleStatus);
  server.on("/builtin/on", turnBuiltinLedOn);
  server.on("/builtin/off", turnBuiltinLedOff);
  server.on("/builtin/toggle", toggleBuiltinLed);
  server.on("/relay1/on", turnRelay1On);
  server.on("/relay1/off", turnRelay1Off);
  server.on("/relay2/on", turnRelay2On);
  server.on("/relay2/off", turnRelay2Off);
  server.on("/mh/digital", readMHSensorDigital);
  server.on("/mh/analog", readMHSensorAnalog);

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle client requests
  server.handleClient();
}