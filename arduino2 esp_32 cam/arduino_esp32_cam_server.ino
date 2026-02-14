// ESP32-CAM (AI Thinker) HTTP server with MJPEG stream and snapshot
// Set your Wi-Fi credentials below. Leave flash off unless needed to avoid glare/heat.

#include <WiFi.h>
#include "esp_camera.h"
#include <WebServer.h>

// Wi-Fi credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const unsigned long STATUS_INTERVAL_MS = 5000; // how often to print Wi-Fi status

// GPIO for the onboard flash LED (AI Thinker uses GPIO 4)
const int FLASH_PIN = 4;

// Camera model: AI Thinker pinout
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WebServer server(80);

unsigned long lastStatusLog = 0;

// Simple control + stream page
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>ESP32-CAM</title>
        <style>
            body { font-family: Arial, sans-serif; background:#0b0c10; color:#e9e9e9; margin:0; padding:16px; }
                h1 { margin:0 0 12px; font-size:22px; }
                    #stream { width:100%; max-width:640px; border:2px solid #1f2833; border-radius:8px; box-shadow:0 4px 18px rgba(0,0,0,0.4); background:#1f2833; }
                        .toolbar { margin:12px 0; display:flex; gap:8px; flex-wrap:wrap; }
                            button { background:#45a29e; color:#0b0c10; border:none; padding:10px 14px; border-radius:6px; font-weight:700; cursor:pointer; }
                                button:active { transform:translateY(1px); }
                                    #snap { width:100%; max-width:320px; display:block; margin-top:10px; border:2px solid #1f2833; border-radius:8px; }
                                        .note { font-size:12px; color:#c5c6c7; }
                                          </style>
                                          </head>
                                          <body>
                                            <h1>ESP32-CAM Live Stream</h1>
                                              <img id="stream" src="/stream" alt="Stream" />
                                                <div class="toolbar">
                                                    <button onclick="toggleFlash(true)">Flash ON</button>
                                                        <button onclick="toggleFlash(false)">Flash OFF</button>
                                                            <button onclick="takePhoto()">Snapshot</button>
                                                              </div>
                                                                <img id="snap" alt="Snapshot preview" />
                                                                  <p class="note">If the stream freezes, reload the page. Keep the flash off for longer sessions.</p>
                                                                  <script>
                                                                  async function toggleFlash(on) {
                                                                    try { await fetch(`/flash?state=${on ? 'on' : 'off'}`); } catch (e) { console.error(e); }
                                                                    }
                                                                    async function takePhoto() {
                                                                      try {
                                                                          const res = await fetch('/capture');
                                                                              if (!res.ok) return;
                                                                                  const blob = await res.blob();
                                                                                      document.getElementById('snap').src = URL.createObjectURL(blob);
                                                                                        } catch (e) { console.error(e); }
                                                                                        }
                                                                                        </script>
                                                                                        </body>
                                                                                        </html>
                                                                                        )rawliteral";

                                                                                        bool initCamera() {
                                                                                          camera_config_t config;
                                                                                            config.ledc_channel = LEDC_CHANNEL_0;
                                                                                              config.ledc_timer = LEDC_TIMER_0;
                                                                                                config.pin_d0 = Y2_GPIO_NUM;
                                                                                                  config.pin_d1 = Y3_GPIO_NUM;
                                                                                                    config.pin_d2 = Y4_GPIO_NUM;
                                                                                                      config.pin_d3 = Y5_GPIO_NUM;
                                                                                                        config.pin_d4 = Y6_GPIO_NUM;
                                                                                                          config.pin_d5 = Y7_GPIO_NUM;
                                                                                                            config.pin_d6 = Y8_GPIO_NUM;
                                                                                                              config.pin_d7 = Y9_GPIO_NUM;
                                                                                                                config.pin_xclk = XCLK_GPIO_NUM;
                                                                                                                  config.pin_pclk = PCLK_GPIO_NUM;
                                                                                                                    config.pin_vsync = VSYNC_GPIO_NUM;
                                                                                                                      config.pin_href = HREF_GPIO_NUM;
                                                                                                                        config.pin_sscb_sda = SIOD_GPIO_NUM;
                                                                                                                          config.pin_sscb_scl = SIOC_GPIO_NUM;
                                                                                                                            config.pin_pwdn = PWDN_GPIO_NUM;
                                                                                                                              config.pin_reset = RESET_GPIO_NUM;
                                                                                                                                config.xclk_freq_hz = 20000000;
                                                                                                                                  config.pixel_format = PIXFORMAT_JPEG;

                                                                                                                                    if (psramFound()) {
                                                                                                                                      config.frame_size = FRAMESIZE_VGA; // original stable settings
                                                                                                                                      config.jpeg_quality = 12;
                                                                                                                                      config.fb_count = 2;
                                                                                                                                    } else {
                                                                                                                                      config.frame_size = FRAMESIZE_QVGA;
                                                                                                                                      config.jpeg_quality = 15;
                                                                                                                                      config.fb_count = 1;
                                                                                                                                    }

                                                                                                                                                                  config.fb_location = CAMERA_FB_IN_PSRAM;
                                                                                                                                                                    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

                                                                                                                                                                      esp_err_t err = esp_camera_init(&config);
                                                                                                                                                                      if (err != ESP_OK) return false;

                                                                                                                                                                      // No sensor tweaks for stability

                                                                                                                                                                      return true;
                                                                                                                                                                        }

                                                                                                                                                                        void handleRoot() { server.send_P(200, "text/html", INDEX_HTML); }

                                                                                                                                                                        void handleCapture() {
                                                                                                                                                                          camera_fb_t* fb = esp_camera_fb_get();
                                                                                                                                                                            if (!fb) {
                                                                                                                                                                                server.send(503, "text/plain", "Camera busy");
                                                                                                                                                                                    return;
                                                                                                                                                                                      }
                                                                                                                                                                                        server.send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);
                                                                                                                                                                                          esp_camera_fb_return(fb);
                                                                                                                                                                                          }

                                                                                                                                                                                          void handleFlash() {
                                                                                                                                                                                            String state = server.arg("state");
                                                                                                                                                                                              bool on = state == "on";
                                                                                                                                                                                                digitalWrite(FLASH_PIN, on ? HIGH : LOW);
                                                                                                                                                                                                  server.send(200, "text/plain", on ? "flash:on" : "flash:off");
                                                                                                                                                                                                  }

                                                                                                                                                                                                  void handleStream() {
                                                                                                                                                                                                    WiFiClient client = server.client();
                                                                                                                                                                                                      if (!client) return;

                                                                                                                                                                                                        String hdr = "HTTP/1.1 200 OK\r\n";
                                                                                                                                                                                                          hdr += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n";
                                                                                                                                                                                                            hdr += "Cache-Control: no-cache\r\n";
                                                                                                                                                                                                              hdr += "Pragma: no-cache\r\n";
                                                                                                                                                                                                                hdr += "Connection: close\r\n\r\n";
                                                                                                                                                                                                                  client.print(hdr);

                                                                                                                                                                                                                    while (client.connected()) {
                                                                                                                                                                                                                        camera_fb_t* fb = esp_camera_fb_get();
                                                                                                                                                                                                                            if (!fb) break;

                                                                                                                                                                                                                                client.print("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: ");
                                                                                                                                                                                                                                    client.print(fb->len);
                                                                                                                                                                                                                                        client.print("\r\n\r\n");
                                                                                                                                                                                                                                            client.write(fb->buf, fb->len);
                                                                                                                                                                                                                                                client.print("\r\n");

                                                                                                                                                                                                                                                    esp_camera_fb_return(fb);
                                                                                                                                                                                                                                                        if (!client.connected()) break;
                                                                                                                                                                                                                                                            delay(5); // throttle slightly
                                                                                                                                                                                                                                                              }
                                                                                                                                                                                                                                                              }

                                                                                                                                                                                                                                                              void handleNotFound() { server.send(404, "text/plain", "Not found"); }

                                                                                                                                                                                                                                                              void connectWiFi() {
                                                                                                                                                                                                                                                                WiFi.mode(WIFI_STA);
                                                                                                                                                                                                                                                                  WiFi.begin(WIFI_SSID, WIFI_PASS);
                                                                                                                                                                                                                                                                    Serial.print("Connecting to WiFi");
                                                                                                                                                                                                                                                                      unsigned long start = millis();
                                                                                                                                                                                                                                                                        while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
                                                                                                                                                                                                                                                                            delay(500);
                                                                                                                                                                                                                                                                                Serial.print('.');
                                                                                                                                                                                                                                                                                  }
                                                                                                                                                                                                                                                                                    Serial.println();
                                                                                                                                                                                                                                                                                      if (WiFi.status() == WL_CONNECTED) {
                                                                                                                                                                                                                                                                                          Serial.print("Connected. IP: ");
                                                                                                                                                                                                                                                                                              Serial.println(WiFi.localIP());
                                                                                                                                                                                                                                                                                                  Serial.print("Open: http://");
                                                                                                                                                                                                                                                                                                      Serial.print(WiFi.localIP());
                                                                                                                                                                                                                                                                                                          Serial.println("/");
                                                                                                                                                                                                                                                                                                            } else {
                                                                                                                                                                                                                                                                                                                Serial.println("WiFi connection failed.");
                                                                                                                                                                                                                                                                                                                  }
                                                                                                                                                                                                                                                                                                                  }

                                                                                                                                                                                                                                                                                                                  void setup() {
                                                                                                                                                                                                                                                                                                                    Serial.begin(115200);
                                                                                                                                                                                                                                                                                                                      delay(200);

                                                                                                                                                                                                                                                                                                                        pinMode(FLASH_PIN, OUTPUT);
                                                                                                                                                                                                                                                                                                                          digitalWrite(FLASH_PIN, LOW);

                                                                                                                                                                                                                                                                                                                            if (!initCamera()) {
                                                                                                                                                                                                                                                                                                                                Serial.println("Camera init failed");
                                                                                                                                                                                                                                                                                                                                    return;
                                                                                                                                                                                                                                                                                                                                      }

                                                                                                                                                                                                                                                                                                                                        connectWiFi();

                                                                                                                                                                                                                                                                                                                                          server.on("/", HTTP_GET, handleRoot);
                                                                                                                                                                                                                                                                                                                                            server.on("/capture", HTTP_GET, handleCapture);
                                                                                                                                                                                                                                                                                                                                              server.on("/flash", HTTP_GET, handleFlash);
                                                                                                                                                                                                                                                                                                                                                server.on("/stream", HTTP_GET, handleStream);
                                                                                                                                                                                                                                                                                                                                                  server.onNotFound(handleNotFound);
                                                                                                                                                                                                                                                                                                                                                    server.begin();

                                                                                                                                                                                                                                                                                                                                                      Serial.println("HTTP server started");
                                                                                                                                                                                                                                                                                                                                                      }

                                                                                                                                                                                                                                                                                                                                                      void loop() {
                                                                                                                                                                                                                                                                                                                                                        server.handleClient();

                                                                                                                                                                                                                                                                                                                                                          unsigned long now = millis();
                                                                                                                                                                                                                                                                                                                                                            if (now - lastStatusLog >= STATUS_INTERVAL_MS) {
                                                                                                                                                                                                                                                                                                                                                                wl_status_t st = WiFi.status();
                                                                                                                                                                                                                                                                                                                                                                    if (st == WL_CONNECTED) {
                                                                                                                                                                                                                                                                                                                                                                          Serial.print("WiFi OK ");
                                                                                                                                                                                                                                                                                                                                                                                Serial.print(WiFi.localIP());
                                                                                                                                                                                                                                                                                                                                                                                      Serial.println(" (open http://<ip>/)");
                                                                                                                                                                                                                                                                                                                                                                                          } else {
                                                                                                                                                                                                                                                                                                                                                                                                Serial.print("WiFi state: ");
                                                                                                                                                                                                                                                                                                                                                                                                      Serial.println((int)st);
                                                                                                                                                                                                                                                                                                                                                                                                          }
                                                                                                                                                                                                                                                                                                                                                                                                              lastStatusLog = now;
                                                                                                                                                                                                                                                                                                                                                                                                                }
                                                                                                                                                                                                                                                                                                                                                                                                                }
                                                                                                                                                                                                                                                                                                                                                                                                                