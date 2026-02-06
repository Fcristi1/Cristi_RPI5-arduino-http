#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Server on port 8080
ESP8266WebServer server(8080);

// Pin configuration
const int LED_PIN = D0;  // GPIO16 - Built-in LED
const int BUTTON_PIN = D1;  // GPIO5

void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  
  // Connect to WiFi
  connectToWiFi();
  
  // Define HTTP routes
  setupRoutes();
  
  // Start server
  server.begin();
  Serial.println("HTTP Server started on port 8080");
}

void loop() {
  server.handleClient();
  
  // Optional: Print button state
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Button pressed!");
    delay(200);
  }
}

void connectToWiFi() {
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Failed to connect to WiFi");
  }
}

void setupRoutes() {
  // GET /status - returns current state
  server.on("/status", HTTP_GET, []() {
    String ledState = digitalRead(LED_PIN) ? "ON" : "OFF";
    String buttonState = digitalRead(BUTTON_PIN) ? "RELEASED" : "PRESSED";
    
    String response = "{\"led\":\"";
    response += ledState;
    response += "\",\"button\":\"";
    response += buttonState;
    response += "\",\"ip\":\"";
    response += WiFi.localIP().toString();
    response += "\"}";
    
    server.send(200, "application/json", response);
  });
  
  // GET /led/on - turn LED on
  server.on("/led/on", HTTP_GET, []() {
    digitalWrite(LED_PIN, HIGH);
    server.send(200, "application/json", "{\"status\":\"LED turned ON\"}");
  });
  
  // GET /led/off - turn LED off
  server.on("/led/off", HTTP_GET, []() {
    digitalWrite(LED_PIN, LOW);
    server.send(200, "application/json", "{\"status\":\"LED turned OFF\"}");
  });
  
  // GET /led/toggle - toggle LED
  server.on("/led/toggle", HTTP_GET, []() {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    String state = digitalRead(LED_PIN) ? "ON" : "OFF";
    String response = "{\"status\":\"LED toggled to ";
    response += state;
    response += "\"}";
    server.send(200, "application/json", response);
  });
  
  // Handle 404
  server.onNotFound([]() {
    server.send(404, "application/json", "{\"error\":\"Not Found\"}");
  });
}
