#!/usr/bin/env python3
"""
RPI5 Web Interface for Arduino D1 WiFi Control
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

app = Flask(__name__)

# Arduino configuration
ARDUINO_IP = "192.168.0.37"
ARDUINO_PORT = 8080
ARDUINO_BASE_URL = f"http://{ARDUINO_IP}:{ARDUINO_PORT}"
REQUEST_TIMEOUT = 5

# Global status
current_status = {
    "led": "OFF",
    "builtin_led": "OFF",
    "button": "RELEASED",
    "ip": "Unknown",
    "connected": False,
    "last_update": None
}

def get_arduino_status():
    """Fetch status from Arduino"""
    global current_status
    try:
        response = requests.get(
            f"{ARDUINO_BASE_URL}/status",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            data = response.json()
            current_status.update({
                "led": data.get("led", "OFF"),
                "builtin_led": data.get("builtin_led", "OFF"),
                "button": data.get("button", "RELEASED"),
                "ip": data.get("ip", "Unknown"),
                "connected": True,
                "last_update": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            })
            return True
        else:
            current_status["connected"] = False
            return False
    except Exception as e:
        current_status["connected"] = False
        print(f"Error fetching status: {e}")
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

@app.route('/')
def index():
    """Serve main page"""
    return render_template('index.html')

@app.route('/api/status', methods=['GET'])
def api_status():
    """API endpoint: get current status"""
    return jsonify(current_status)

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
            f"{ARDUINO_BASE_URL}/led/on",
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
            f"{ARDUINO_BASE_URL}/led/off",
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
            f"{ARDUINO_BASE_URL}/led/toggle",
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
            f"{ARDUINO_BASE_URL}/builtin/on",
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
            f"{ARDUINO_BASE_URL}/builtin/off",
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
            f"{ARDUINO_BASE_URL}/builtin/toggle",
            timeout=REQUEST_TIMEOUT
        )
        if response.status_code == 200:
            get_arduino_status()
            return jsonify({"status": "success", "action": "Built-in LED toggled"})
        else:
            return jsonify({"status": "error", "message": "Arduino returned error"}), 500
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
    print(f"Arduino IP configured as: {ARDUINO_IP}:{ARDUINO_PORT}")
    print(f"Web interface will be available at: http://<RPI5_IP>:5000")
    print(f"Access from PC: http://192.168.x.x:5000 (replace with your RPI5 IP)")
    print(f"Press Ctrl+C to stop the server\n")
    
    # Initial connection test
    if get_arduino_status():
        print(f"✓ Connected to Arduino at {ARDUINO_IP}")
    else:
        print(f"✗ Could not connect to Arduino at {ARDUINO_IP}")
        print(f"  You can change the IP in the web interface settings")
    
    print(f"{'='*50}\n")
    app.run(host='0.0.0.0', port=5000, debug=False)
