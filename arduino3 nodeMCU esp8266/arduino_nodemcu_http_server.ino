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

  // Define server routes
  server.on("/", handleRoot);
  server.on("/toggleChannel1", toggleChannel1);
  server.on("/toggleChannel2", toggleChannel2);

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle client requests
  server.handleClient();
}