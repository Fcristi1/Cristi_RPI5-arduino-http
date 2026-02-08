#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "Mimi";
const char* password = "mimimimi1";

// Server on port 8080
ESP8266WebServer server(8080);

// Pin configuration
const int LED_PIN = D0;  // GPIO16 - External LED
// Use the predefined LED_BUILTIN macro for the built-in LED
const int BUILTIN_LED_PIN = LED_BUILTIN;  // Ensures compatibility with the board
const int BUTTON_PIN = D1;  // GPIO5
const int DHTPIN = D11;  // Pin connected to the DHT sensor
const int DHTTYPE = DHT11;
DHT dht(DHTPIN, DHTTYPE);

// Define calibration offsets for temperature and humidity
const float TEMP_OFFSET = 1.2;  // Adjusted to match thermometer
const float HUMIDITY_OFFSET = 5.0;  // Adjusted to match thermometer

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUILTIN_LED_PIN, OUTPUT); // Initialize the built-in LED as an output

  // Test the built-in LED
  digitalWrite(BUILTIN_LED_PIN, LOW);  // Turn ON the LED (active LOW)
  delay(1000);
  digitalWrite(BUILTIN_LED_PIN, HIGH); // Turn OFF the LED
  delay(1000);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUILTIN_LED_PIN, HIGH); // Off by default (active LOW)

  // Connect to WiFi
  connectToWiFi();

  // Initialize DHT sensor
  dht.begin();
  Serial.println("DHT sensor initialized on D11");

  // Define HTTP routes
  setupRoutes();

  // Start server
  server.begin();
  Serial.println("HTTP Server started on port 8080");
}

void loop() {
  server.handleClient();

  // Button debounce logic
  static unsigned long lastPress = 0;
  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long currentTime = millis();
    if (currentTime - lastPress > 200) { // 200ms debounce
      Serial.println("Button pressed!");
      lastPress = currentTime;
    }
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

    float temperature = dht.readTemperature() + TEMP_OFFSET;
    float humidity = dht.readHumidity() + HUMIDITY_OFFSET;

    String response = "{\"led\":\"";
    response += ledState;
    response += "\",\"button\":\"";
    response += buttonState;
    response += "\",\"builtin_led\":\"";
    response += builtinLedState;
    response += "\",\"ip\":\"";
    response += WiFi.localIP().toString();

    if (!isnan(temperature) && !isnan(humidity)) {
      response += "\",\"temperature\":\"";
      response += String(temperature);
      response += "\",\"humidity\":\"";
      response += String(humidity);
    }

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

  // GET /sensor - returns calibrated temperature and humidity
  server.on("/sensor", HTTP_GET, []() {
    float temperature = dht.readTemperature() + TEMP_OFFSET;
    float humidity = dht.readHumidity() + HUMIDITY_OFFSET;

    if (isnan(temperature) || isnan(humidity)) {
      server.send(500, "application/json", "{\"error\":\"Failed to read from DHT sensor\"}");
      return;
    }

    char response[100];
    snprintf(response, sizeof(response), "{\"temperature\":%.1f,\"humidity\":%.1f}", temperature, humidity);

    server.send(200, "application/json", response);
  });

  // GET /button - returns the current button state
  server.on("/button", HTTP_GET, []() {
    String buttonState = digitalRead(BUTTON_PIN) == LOW ? "PRESSED" : "RELEASED";
    String response = "{\"button\":\"" + buttonState + "\"}";
    server.send(200, "application/json", response);
  });  // Handle 404
  server.onNotFound([]() {
    server.send(404, "application/json", "{\"error\":\"Not Found\"}");
  });
}
