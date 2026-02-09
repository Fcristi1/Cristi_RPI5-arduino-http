#!/usr/bin/env python3
"""
RPI5 Web Interface for Arduino Devices
Flask-based web interface accessible from PC browser
Run on RPI5: python3 rpi5/web_server.py
Access from PC: http://<RPI5_IP>:5000
"""

from flask import Flask, render_template, jsonify, request
import requests
import json
import threading
from datetime import datetime
import os

# Configure Flask to serve templates from parent directory
app = Flask(__name__, template_folder='../templates', static_folder='../static')

# Arduino configuration for renamed folders
ARDUINO_1_IP = "192.168.0.37"  # Arduino 1 (D1 ESP8266)
ARDUINO_3_IP = "192.168.0.161"  # Arduino 3 (NodeMCU ESP8266)

# Use distinct ports: Arduino 1 listens on 8080, Arduino 3 (NodeMCU) on the default HTTP port 80
ARDUINO_1_PORT = 8080
ARDUINO_3_PORT = 80

ARDUINO_1_BASE_URL = f"http://{ARDUINO_1_IP}:{ARDUINO_1_PORT}"
ARDUINO_3_BASE_URL = f"http://{ARDUINO_3_IP}:{ARDUINO_3_PORT}"
# Legacy variable kept for the config endpoint (assumes Arduino 1)
ARDUINO_PORT = ARDUINO_1_PORT
REQUEST_TIMEOUT = 5

# Global status
current_status = {
    "led": "OFF",
    "builtin_led": "OFF",
    "ip": "Unknown",
    "connected": False,
    "last_update": None
}

def get_arduino_status():
    """Fetch status from Arduino"""
    global current_status
    try:
        response = requests.get(
            f"{ARDUINO_1_BASE_URL}/status",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            data = response.json()
            current_status.update({
                "led": data.get("led", "OFF"),
                "builtin_led": data.get("builtin_led", "OFF"),
                "ip": data.get("ip", "Unknown"),
                "connected": data.get("builtin_led", "OFF") == "ON",  # Online if Built-in LED is ON
                "last_update": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            })
            print("Successfully fetched status from Arduino 1:", current_status)
            return True
        else:
            current_status["connected"] = False
            print(f"Failed to fetch status from Arduino 1: HTTP {response.status_code}")
            return False
    except Exception as e:
        current_status["connected"] = False
        print(f"Error fetching status from Arduino 1: {e}")
        return False

def continuous_status_update():
    """Background thread to continuously update status"""
    while True:
        get_arduino_status()
        import time
        time.sleep(1)  # Update every 1 second

# Start background thread
status_thread = threading.Thread(target=continuous_status_update, daemon=True)
status_thread.start()

def get_arduino1_status():
    """Fetch status from Arduino 1 (D1)"""
    try:
        response = requests.get(
            f"{ARDUINO_1_BASE_URL}/status",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            data = response.json()
            return {
                "led": data.get("led", "OFF"),
                "builtin_led": data.get("builtin_led", "OFF"),
                "temperature": data.get("temperature", "N/A"),
                "humidity": data.get("humidity", "N/A"),
                "ip": ARDUINO_1_IP,
                "last_update": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            }
        else:
            return {
                "error": f"Failed to fetch status: HTTP {response.status_code}"
            }
    except Exception as e:
        return {
            "error": str(e)
        }

def get_arduino3_status():
    """Fetch status from Arduino 3 (NodeMCU)"""
    try:
        response = requests.get(
            f"{ARDUINO_3_BASE_URL}/status",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            data = response.json()
            # Normalize keys coming from NodeMCU sketch
            return {
                "builtin_led": data.get("builtin_led", "OFF"),
                "relay_channel_1": data.get("relay1", data.get("relay_channel_1", "OFF")),
                "relay_channel_2": data.get("relay2", data.get("relay_channel_2", "OFF")),
                "ip": ARDUINO_3_IP,
                "last_update": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            }
        else:
            return {
                "error": f"Failed to fetch status: HTTP {response.status_code}"
            }
    except Exception as e:
        return {
            "error": str(e)
        }

def get_nodemcu_status():
    """Fetch the status of Arduino 3 (NodeMCU)."""
    try:
        response = requests.get(f"{ARDUINO_3_BASE_URL}/status", timeout=5)
        if response.status_code == 200:
            return response.json()
        else:
            return {"error": "Failed to fetch NodeMCU status"}
    except requests.exceptions.RequestException as e:
        return {"error": str(e)}

@app.route('/api/arduino1/led/toggle', methods=['POST'])
def toggle_arduino1_led():
    """Toggle Arduino 1 built-in LED"""
    try:
        print(f"[DEBUG] Attempting to toggle built-in LED on {ARDUINO_1_BASE_URL}/builtin/toggle")
        response = requests.get(f"{ARDUINO_1_BASE_URL}/builtin/toggle", timeout=REQUEST_TIMEOUT)
        print(f"[DEBUG] Arduino response status: {response.status_code}")
        if response.status_code == 200:
            return jsonify(response.json())
        else:
            print(f"[ERROR] Arduino returned error: {response.status_code}")
            return jsonify({"error": "Failed to toggle built-in LED"}), 500
    except Exception as e:
        print(f"[ERROR] Exception while toggling built-in LED: {str(e)}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/arduino3/led/toggle', methods=['POST'])
def toggle_arduino3_led():
    """Toggle Arduino 3 LED"""
    try:
        response = requests.get(f"{ARDUINO_3_BASE_URL}/builtin/toggle", timeout=REQUEST_TIMEOUT)
        if response.status_code == 200:
            return jsonify(response.json())
        else:
            return jsonify({"error": "Failed to toggle LED"}), 500
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/')
def index():
    """Serve main page"""
    return render_template('index.html')

@app.route('/api/arduino1/status', methods=['GET'])
def arduino1_status():
    """Fetch Arduino 1 status"""
    try:
        response = requests.get(f"{ARDUINO_1_BASE_URL}/status", timeout=REQUEST_TIMEOUT)
        if response.status_code != 200:
            return jsonify({"error": "Failed to fetch status"}), 500

        status_payload = response.json()

        # Fill in temperature/humidity by calling the dedicated sensor endpoint when missing
        if "temperature" not in status_payload or "humidity" not in status_payload:
            try:
                sensor_resp = requests.get(f"{ARDUINO_1_BASE_URL}/sensor", timeout=REQUEST_TIMEOUT)
                if sensor_resp.status_code == 200:
                    sensor_data = sensor_resp.json()
                    status_payload["temperature"] = sensor_data.get("temperature", "-")
                    status_payload["humidity"] = sensor_data.get("humidity", "-")
            except Exception as sensor_err:
                print(f"[WARN] Failed to augment Arduino1 status with sensor data: {sensor_err}")

        return jsonify(status_payload)
    except Exception as e:
        return jsonify({"error": str(e), "connected": False}), 500

@app.route('/api/arduino3/status', methods=['GET'])
def arduino3_status():
    """Fetch Arduino 3 status"""
    try:
        response = requests.get(f"{ARDUINO_3_BASE_URL}/status", timeout=REQUEST_TIMEOUT)
        if response.status_code == 200:
            data = response.json()
            return jsonify({
                "builtin_led": data.get("builtin_led", "OFF"),
                "relay_channel_1": data.get("relay1", data.get("relay_channel_1", "OFF")),
                "relay_channel_2": data.get("relay2", data.get("relay_channel_2", "OFF")),
                "ip": ARDUINO_3_IP,
                "last_update": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            })
        else:
            return jsonify({"error": "Failed to fetch status"}), 500
    except Exception as e:
        return jsonify({"error": str(e), "connected": False}), 500

@app.route('/api/nodemcu/status', methods=['GET'])
def nodemcu_status():
    """Fetch Arduino 3 status for legacy frontend endpoint."""
    try:
        response = requests.get(f"{ARDUINO_3_BASE_URL}/status", timeout=REQUEST_TIMEOUT)
        if response.status_code == 200:
            data = response.json()
            return jsonify({
                "builtin_led": data.get("builtin_led", "OFF"),
                "relay_channel_1": data.get("relay1", data.get("relay_channel_1", "OFF")),
                "relay_channel_2": data.get("relay2", data.get("relay_channel_2", "OFF")),
                "ip": ARDUINO_3_IP,
                "last_update": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            })
        return jsonify({"error": "Failed to fetch status"}), 500
    except Exception as e:
        return jsonify({"error": str(e), "connected": False}), 500

@app.route('/api/arduino3/mh', methods=['GET'])
def arduino3_mh():
    """Fetch MH sensor data from Arduino 3"""
    try:
        digital_response = requests.get(f"{ARDUINO_3_BASE_URL}/mh/digital", timeout=REQUEST_TIMEOUT)
        analog_response = requests.get(f"{ARDUINO_3_BASE_URL}/mh/analog", timeout=REQUEST_TIMEOUT)

        if digital_response.status_code == 200 and analog_response.status_code == 200:
            digital_data = digital_response.json()
            analog_data = analog_response.json()
            return jsonify({
                "digital": digital_data.get("digital", "N/A"),
                "analog": analog_data.get("analog", "N/A")
            })

        return jsonify({"error": "Failed to fetch MH sensor"}), 500
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/arduino3/relay1/on', methods=['POST'])
def arduino3_relay1_on():
    try:
        response = requests.get(f"{ARDUINO_3_BASE_URL}/relay1/on", timeout=REQUEST_TIMEOUT)
        if response.status_code == 200:
            return jsonify(response.json())
        return jsonify({"error": "Failed to turn Relay 1 ON"}), 500
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/arduino3/relay1/off', methods=['POST'])
def arduino3_relay1_off():
    try:
        response = requests.get(f"{ARDUINO_3_BASE_URL}/relay1/off", timeout=REQUEST_TIMEOUT)
        if response.status_code == 200:
            return jsonify(response.json())
        return jsonify({"error": "Failed to turn Relay 1 OFF"}), 500
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/arduino3/relay2/on', methods=['POST'])
def arduino3_relay2_on():
    try:
        response = requests.get(f"{ARDUINO_3_BASE_URL}/relay2/on", timeout=REQUEST_TIMEOUT)
        if response.status_code == 200:
            return jsonify(response.json())
        return jsonify({"error": "Failed to turn Relay 2 ON"}), 500
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/arduino3/relay2/off', methods=['POST'])
def arduino3_relay2_off():
    try:
        response = requests.get(f"{ARDUINO_3_BASE_URL}/relay2/off", timeout=REQUEST_TIMEOUT)
        if response.status_code == 200:
            return jsonify(response.json())
        return jsonify({"error": "Failed to turn Relay 2 OFF"}), 500
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/status', methods=['GET'])
def get_status():
    """Fetch the status of both Arduino 1 and Arduino 3."""
    try:
        # Fetch Arduino 1 status
        arduino1_status = requests.get(f"{ARDUINO_1_BASE_URL}/status", timeout=REQUEST_TIMEOUT).json()
    except Exception as e:
        arduino1_status = {"error": str(e)}

    try:
        # Fetch Arduino 3 status
        raw_arduino3 = requests.get(f"{ARDUINO_3_BASE_URL}/status", timeout=REQUEST_TIMEOUT).json()
        arduino3_status = {
            "builtin_led": raw_arduino3.get("builtin_led", "OFF"),
            "relay_channel_1": raw_arduino3.get("relay1", raw_arduino3.get("relay_channel_1", "OFF")),
            "relay_channel_2": raw_arduino3.get("relay2", raw_arduino3.get("relay_channel_2", "OFF")),
            "ip": ARDUINO_3_IP,
            "last_update": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        }
    except Exception as e:
        arduino3_status = {"error": str(e)}

    # Combine statuses
    return jsonify({
        "arduino1": arduino1_status,
        "arduino3": arduino3_status
    })

@app.route('/api/config', methods=['GET'])
def api_config():
    """API endpoint: get configuration"""
    return jsonify({
        "arduino_ip": ARDUINO_IP,
        "arduino_port": ARDUINO_PORT,
        "request_timeout": REQUEST_TIMEOUT
    })

@app.route('/api/config', methods=['POST'])
def api_set_config():
    """API endpoint: set Arduino IP"""
    global ARDUINO_IP, ARDUINO_BASE_URL
    data = request.get_json()
    
    if 'ip' in data:
        ARDUINO_IP = data['ip']
        ARDUINO_BASE_URL = f"http://{ARDUINO_IP}:{ARDUINO_PORT}"
        # Try to connect immediately
        get_arduino_status()
        return jsonify({
            "status": "success",
            "message": f"Arduino IP updated to {ARDUINO_IP}",
            "connected": current_status["connected"]
        })
    
    return jsonify({"status": "error", "message": "Invalid configuration"}), 400

@app.route('/api/led/on', methods=['POST'])
def api_led_on():
    """API endpoint: turn LED on"""
    try:
        response = requests.get(
            f"{ARDUINO_1_BASE_URL}/led/on",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            get_arduino_status()
            return jsonify({"status": "success", "action": "LED turned ON"})
        else:
            return jsonify({"status": "error", "message": "Arduino returned error"}), 500
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/led/off', methods=['POST'])
def api_led_off():
    """API endpoint: turn LED off"""
    try:
        response = requests.get(
            f"{ARDUINO_1_BASE_URL}/led/off",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            get_arduino_status()
            return jsonify({"status": "success", "action": "LED turned OFF"})
        else:
            return jsonify({"status": "error", "message": "Arduino returned error"}), 500
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/led/toggle', methods=['POST'])
def api_led_toggle():
    """API endpoint: toggle LED"""
    try:
        response = requests.get(
            f"{ARDUINO_1_BASE_URL}/led/toggle",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            get_arduino_status()
            return jsonify({"status": "success", "action": "LED toggled"})
        else:
            return jsonify({"status": "error", "message": "Arduino returned error"}), 500
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/builtin/on', methods=['POST'])
def api_builtin_on():
    """API endpoint: turn built-in LED on"""
    try:
        response = requests.get(
            f"{ARDUINO_1_BASE_URL}/builtin/on",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            get_arduino_status()
            return jsonify({"status": "success", "action": "Built-in LED turned ON"})
        else:
            return jsonify({"status": "error", "message": "Arduino returned error"}), 500
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/builtin/off', methods=['POST'])
def api_builtin_off():
    """API endpoint: turn built-in LED off"""
    try:
        response = requests.get(
            f"{ARDUINO_1_BASE_URL}/builtin/off",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            get_arduino_status()
            return jsonify({"status": "success", "action": "Built-in LED turned OFF"})
        else:
            return jsonify({"status": "error", "message": "Arduino returned error"}), 500
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/builtin/toggle', methods=['POST'])
def api_builtin_toggle():
    """API endpoint: toggle built-in LED"""
    try:
        response = requests.get(
            f"{ARDUINO_1_BASE_URL}/builtin/toggle",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            get_arduino_status()
            return jsonify({"status": "success", "action": "Built-in LED toggled"})
        else:
            return jsonify({"status": "error", "message": "Arduino returned error"}), 500
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/sensor', methods=['GET'])
def api_sensor():
    """API endpoint: get temperature and humidity from Arduino"""
    try:
        response = requests.get(
            f"{ARDUINO_1_BASE_URL}/sensor",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            return jsonify(response.json())
        else:
            return jsonify({"status": "error", "message": "Arduino returned error"}), 500
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/temperature', methods=['GET'])
def api_temperature():
    """API endpoint: get temperature from Arduino"""
    try:
        response = requests.get(
            f"{ARDUINO_1_BASE_URL}/sensor",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            data = response.json()
            return jsonify({"temperature": data.get("temperature", "N/A")})
        else:
            return jsonify({"status": "error", "message": "Arduino returned error"}), 500
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/d1/status', methods=['GET'])
def d1_status():
    """API endpoint: Get status from Arduino D1"""
    try:
        response = requests.get(f"{ARDUINO_1_BASE_URL}/status", timeout=REQUEST_TIMEOUT)
        if response.status_code == 200:
            return response.json(), 200
        else:
            return jsonify({"error": "Failed to fetch status from Arduino D1"}), response.status_code
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/nodemcu/status', methods=['GET'])
def api_nodemcu_status():
    """API endpoint to get NodeMCU status"""
    return get_nodemcu_status()

@app.route('/api/nodemcu/toggle/<int:channel>', methods=['POST'])
def toggle_channel(channel):
    """API endpoint: Toggle a specific channel on NodeMCU"""
    if channel not in [1, 2]:
        return jsonify({"error": "Invalid channel"}), 400
    try:
        response = requests.get(f"{ARDUINO_BASE_URL}/toggleChannel{channel}", timeout=REQUEST_TIMEOUT)
        if response.status_code == 200:
            return response.text, 200  # Return plain text response
        else:
            return jsonify({"error": "Failed to toggle channel"}), response.status_code
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/api/arduino3/sensor', methods=['GET'])
def get_arduino3_sensor():
    """Fetch sensor data from Arduino 3"""
    try:
        response = requests.get(f"{ARDUINO_3_BASE_URL}/sensor", timeout=REQUEST_TIMEOUT)
        if response.status_code == 200:
            return jsonify(response.json())
        else:
            return jsonify({"status": "error", "message": "Failed to fetch sensor data"}), 500
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.errorhandler(404)
def not_found(error):
    """Handle 404 errors"""
    return jsonify({"error": "Not found"}), 404

@app.errorhandler(500)
def internal_error(error):
    """Handle 500 errors"""
    return jsonify({"error": "Internal server error"}), 500

if __name__ == '__main__':
    print(f"\n{'='*50}")
    print(f"RPI5 Web Interface - Arduino D1 Control")
    print(f"{'='*50}")
    print(f"Arduino IP configured as: {ARDUINO_1_IP}:{ARDUINO_PORT}")
    print(f"Web interface will be available at: http://<RPI5_IP>:5000")
    print(f"Access from PC: http://192.168.x.x:5000 (replace with your RPI5 IP)")
    print(f"Press Ctrl+C to stop the server\n")
    
    # Initial connection test
    if get_arduino_status():
        print(f"✓ Connected to Arduino at {ARDUINO_1_IP}")
    else:
        print(f"✗ Could not connect to Arduino at {ARDUINO_1_IP}")
        print(f"  You can change the IP in the web interface settings")
    
    print(f"{'='*50}\n")
    app.run(host='0.0.0.0', port=5000, debug=False)
