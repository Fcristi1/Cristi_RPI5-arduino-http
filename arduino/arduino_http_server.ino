#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "Mimimimi1";
const char* password = "mimimimi1";

// Server on port 8080
ESP8266WebServer server(8080);

// Pin configuration
const int LED_PIN = D0;  // GPIO16 - External LED
const int BUILTIN_LED_PIN = D4;  // GPIO2 - On-board LED (active LOW)
const int BUTTON_PIN = D1;  // GPIO5
const int DHTPIN = D2;  // Pin connected to the DHT sensor
const int DHTTYPE = DHT11;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUILTIN_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUILTIN_LED_PIN, HIGH); // Off by default (active LOW)

  // Connect to WiFi
  connectToWiFi();

  // Initialize DHT sensor
  dht.begin();

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
    String builtinLedState = (digitalRead(BUILTIN_LED_PIN) == LOW) ? "ON" : "OFF";

    String response = "{\"led\":\"";
    response += ledState;
    response += "\",\"button\":\"";
    response += buttonState;
    response += "\",\"builtin_led\":\"";
    response += builtinLedState;
    response += "\",\"ip\":\"";
    response += WiFi.localIP().toString();
    response += "\"}";

    server.send(200, "application/json", response);
  });

  // GET /led/on - turn external LED on
  server.on("/led/on", HTTP_GET, []() {
    digitalWrite(LED_PIN, HIGH);
    server.send(200, "application/json", "{\"status\":\"LED turned ON\"}");
  });

  // GET /led/off - turn external LED off
  server.on("/led/off", HTTP_GET, []() {
    digitalWrite(LED_PIN, LOW);
    server.send(200, "application/json", "{\"status\":\"LED turned OFF\"}");
  });

  // GET /led/toggle - toggle external LED
  server.on("/led/toggle", HTTP_GET, []() {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    String state = digitalRead(LED_PIN) ? "ON" : "OFF";
    String response = "{\"status\":\"LED toggled to ";
    response += state;
    response += "\"}";
    server.send(200, "application/json", response);
  });

  // GET /builtin/on - turn built-in LED on (active LOW)
  server.on("/builtin/on", HTTP_GET, []() {
    digitalWrite(BUILTIN_LED_PIN, LOW);
    server.send(200, "application/json", "{\"status\":\"Built-in LED turned ON\"}");
  });

  // GET /builtin/off - turn built-in LED off (active LOW)
  server.on("/builtin/off", HTTP_GET, []() {
    digitalWrite(BUILTIN_LED_PIN, HIGH);
    server.send(200, "application/json", "{\"status\":\"Built-in LED turned OFF\"}");
  });

  // GET /builtin/toggle - toggle built-in LED
  server.on("/builtin/toggle", HTTP_GET, []() {
    bool isOn = (digitalRead(BUILTIN_LED_PIN) == LOW);
    digitalWrite(BUILTIN_LED_PIN, isOn ? HIGH : LOW);
    String state = isOn ? "OFF" : "ON";
    String response = "{\"status\":\"Built-in LED toggled to ";
    response += state;
    response += "\"}";
    server.send(200, "application/json", response);
  });

  // GET /sensor - returns temperature and humidity
  server.on("/sensor", HTTP_GET, []() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      server.send(500, "application/json", "{\"error\":\"Failed to read from DHT sensor\"}");
      return;
    }

    String response = "{\"temperature\":";
    response += temperature;
    response += ",\"humidity\":";
    response += humidity;
    response += "}";

    server.send(200, "application/json", response);
  });

  // Handle 404
  server.onNotFound([]() {
    server.send(404, "application/json", "{\"error\":\"Not Found\"}");
  });
}
