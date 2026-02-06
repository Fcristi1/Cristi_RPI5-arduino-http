#!/usr/bin/env python3
"""
RPI5 HTTP Client for Arduino D1 WiFi Communication
"""

import requests
import json
import time
import sys
from datetime import datetime

class ArduinoClient:
    def __init__(self, arduino_ip, arduino_port=8080):
        """Initialize Arduino client"""
        self.base_url = f"http://{arduino_ip}:{arduino_port}"
        self.timeout = 5
        
    def is_connected(self):
        """Check if Arduino is reachable"""
        try:
            response = requests.get(f"{self.base_url}/status", timeout=self.timeout)
            return response.status_code == 200
        except:
            return False
    
    def get_status(self):
        """Get current status from Arduino"""
        try:
            response = requests.get(f"{self.base_url}/status", timeout=self.timeout)
            if response.status_code == 200:
                return response.json()
            else:
                print(f"Error: HTTP {response.status_code}")
                return None
        except requests.exceptions.ConnectionError:
            print(f"Error: Cannot connect to Arduino at {self.base_url}")
            return None
        except Exception as e:
            print(f"Error: {e}")
            return None
    
    def led_on(self):
        """Turn LED on"""
        try:
            response = requests.get(f"{self.base_url}/led/on", timeout=self.timeout)
            return response.json() if response.status_code == 200 else None
        except Exception as e:
            print(f"Error: {e}")
            return None
    
    def led_off(self):
        """Turn LED off"""
        try:
            response = requests.get(f"{self.base_url}/led/off", timeout=self.timeout)
            return response.json() if response.status_code == 200 else None
        except Exception as e:
            print(f"Error: {e}")
            return None
    
    def led_toggle(self):
        """Toggle LED"""
        try:
            response = requests.get(f"{self.base_url}/led/toggle", timeout=self.timeout)
            return response.json() if response.status_code == 200 else None
        except Exception as e:
            print(f"Error: {e}")
            return None

def main():
    """Interactive CLI for Arduino control"""
    
    # Get Arduino IP from argument or prompt
    if len(sys.argv) > 1:
        arduino_ip = sys.argv[1]
    else:
        arduino_ip = input("Enter Arduino IP address: ")
    
    client = ArduinoClient(arduino_ip)
    
    print(f"\nConnecting to Arduino at {arduino_ip}...")
    if not client.is_connected():
        print("Error: Cannot reach Arduino. Check IP and WiFi connection.")
        sys.exit(1)
    
    print("✓ Connected to Arduino!\n")
    
    # Get initial status
    status = client.get_status()
    if status:
        print(f"Arduino Status: {json.dumps(status, indent=2)}\n")
    
    # Interactive menu
    while True:
        print("Commands:")
        print("  1 - Get status")
        print("  2 - LED on")
        print("  3 - LED off")
        print("  4 - LED toggle")
        print("  5 - Exit")
        
        choice = input("\nEnter command (1-5): ").strip()
        
        if choice == "1":
            status = client.get_status()
            if status:
                print(f"\nStatus: {json.dumps(status, indent=2)}")
        elif choice == "2":
            print("Turning LED on...")
            result = client.led_on()
            print(f"Result: {result}")
        elif choice == "3":
            print("Turning LED off...")
            result = client.led_off()
            print(f"Result: {result}")
        elif choice == "4":
            print("Toggling LED...")
            result = client.led_toggle()
            print(f"Result: {result}")
        elif choice == "5":
            print("Exiting...")
            break
        else:
            print("Invalid command")
        
        print()

if __name__ == "__main__":
    main()
