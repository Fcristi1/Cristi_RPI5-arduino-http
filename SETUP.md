# RPI5 + Arduino D1 WiFi HTTP Connection Setup

## Overview
This project establishes HTTP communication between a Raspberry Pi 5 and Arduino D1 WiFi board:
- **Arduino D1**: Runs HTTP server on port 8080
- **RPI5**: Runs Flask web interface on port 5000 + Python commands to control Arduino
- **PC**: Accesses web interface via RPI5 IP address

---

## Prerequisites

### Arduino Side
1. Arduino IDE or Arduino CLI installed
2. ESP8266 board package installed in Arduino IDE
3. Libraries needed:
   - `ESP8266WiFi` (included with ESP8266 package)
   - `ESP8266WebServer` (included with ESP8266 package)

### RPI5 Side
1. Python 3 installed
2. Dependencies: `pip install -r rpi5/requirements.txt`

---

## Step 1: Configure Arduino Sketch

**File:** `arduino/arduino_http_server.ino`

Edit these lines with your WiFi credentials:
```cpp
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
```

**Pin Setup (customize as needed):**
- `LED_PIN = D0` (GPIO16) - LED control
- `BUTTON_PIN = D1` (GPIO5) - Button input

### Upload to Arduino D1
1. Open Arduino IDE
2. Go to Tools → Board → ESP8266 Boards → LOLIN(WEMOS) D1 R2 & mini
3. Connect D1 via USB
4. Select correct COM port
5. Click Upload

After upload, open Serial Monitor (115200 baud) to see:
- WiFi connection status
- Arduino's IP address (e.g., `192.168.1.100`)
- Server startup confirmation

**Note your Arduino's IP address!**

---

## Step 2: Setup RPI5 Web Interface

### Install Dependencies
```bash
cd /path/to/Cristi_RPI5-arduino-http
pip install -r rpi5/requirements.txt
```

### Run Web Server
```bash
python3 rpi5/web_server.py
```

You'll see output like:
```
==================================================
RPI5 Web Interface - Arduino D1 Control
==================================================
Arduino IP configured as: 192.168.1.100:8080
Web interface will be available at: http://<RPI5_IP>:5000
Access from PC: http://192.168.x.x:5000 (replace with your RPI5 IP)
Press Ctrl+C to stop the server

✓ Connected to Arduino at 192.168.1.100
==================================================
```

### Access from PC
1. Find RPI5 IP: On RPI5 terminal, run `hostname -I`
2. Open web browser on your PC
3. Navigate to: `http://<RPI5_IP>:5000` (e.g., `http://192.168.1.50:5000`)
4. Control Arduino D1 from the web interface!

---

## Features of Web Interface

### Real-time Status Display
- Connection status to Arduino
- Current LED state (ON/OFF)
- Button input state (PRESSED/RELEASED)
- Arduino IP address and last update time

### LED Control
- Turn LED ON
- Turn LED OFF
- Toggle LED

### Button Monitoring
- Real-time button state display
- Updates every 1 second

### Settings
- Change Arduino IP address on the fly
- No need to restart the server

### Keyboard Shortcuts
- **Ctrl+1** or **Cmd+1**: LED ON
- **Ctrl+0** or **Cmd+0**: LED OFF
- **Ctrl+T** or **Cmd+T**: Toggle LED

---

## Files Structure
```
/
├── arduino/
│   └── arduino_http_server.ino          # Arduino D1 code
├── rpi5/
│   ├── web_server.py                    # Flask web server (run on RPI5)
│   ├── arduino_client.py                # CLI client (alternative)
│   ├── requirements.txt                 # Python dependencies
│   ├── templates/
│   │   └── index.html                   # Web interface HTML
│   └── static/
│       ├── style.css                    # Web interface styling
│       └── script.js                    # Web interface JavaScript
├── config.env                           # Configuration template
├── SETUP.md                             # This file
└── README.md
```

---

## Step 3: Alternative - CLI Client

If you prefer command-line control on RPI5:

```bash
# Interactive mode
python3 rpi5/arduino_client.py

# Specify IP directly
python3 rpi5/arduino_client.py 192.168.1.100
```

Then use the menu to control the Arduino.

---

## Testing Connection

### From RPI5 Terminal
```bash
# Test Arduino connectivity
curl http://192.168.1.100:8080/status

# Turn LED on
curl http://192.168.1.100:8080/led/on

# Turn LED off
curl http://192.168.1.100:8080/led/off
```

### From Web Interface
1. Open browser at `http://<RPI5_IP>:5000`
2. Check "Connection Status" shows "Connected" (green dot)
3. Click "Turn ON" button
4. Observe LED on Arduino turns on
5. Click "Turn OFF" button
6. Observe LED turns off

---

## Available HTTP Endpoints

The Arduino D1 exposes these endpoints:

### `GET /status`
Returns JSON with current state:
```json
{
  "led": "ON",
  "button": "RELEASED",
  "ip": "192.168.1.100"
}
```

### `GET /led/on`
Turns LED on

### `GET /led/off`
Turns LED off

### `GET /led/toggle`
Toggles LED state

---

## Troubleshooting

### Arduino not connecting to WiFi
- Check SSID and password are correct in sketch
- Ensure 2.4GHz WiFi (D1 doesn't support 5GHz)
- Check WiFi is in range

### RPI5 can't reach Arduino
- Verify both on same WiFi network
- Check Arduino's IP address in Serial Monitor
- Test: `ping 192.168.1.100` from RPI5
- Check firewall isn't blocking port 8080

### Web interface not accessible from PC
- Verify RPI5 IP address: `hostname -I` on RPI5
- Test RPI5 connectivity: `ping <RPI5_IP>` from PC
- Check firewall isn't blocking port 5000
- Try `curl http://<RPI5_IP>:5000` from PC terminal

### Web interface shows "Disconnected"
- Check Arduino IP in Settings on web interface
- Verify Arduino is running and connected to WiFi
- Check Serial Monitor on Arduino for errors

---

## Advanced: Customization

### Add More GPIO Control
Edit `arduino/arduino_http_server.ino` to add new endpoints:
```cpp
server.on("/pin/d5/high", HTTP_GET, []() {
  digitalWrite(D5, HIGH);
  server.send(200, "application/json", "{\"status\":\"D5 set HIGH\"}");
});
```

### Change Web Server Port
Edit `rpi5/web_server.py`:
```python
app.run(host='0.0.0.0', port=8000, debug=False)  # Change 5000 to desired port
```

### Enable Debug Mode
Edit `rpi5/web_server.py`:
```python
app.run(host='0.0.0.0', port=5000, debug=True)  # Set debug=True for development
```

---

## Next Steps
1. ✓ Upload Arduino code and note its IP
2. ✓ Install Python dependencies on RPI5
3. ✓ Run web server on RPI5
4. ✓ Access web interface from PC
5. ✓ Control Arduino from web interface!
