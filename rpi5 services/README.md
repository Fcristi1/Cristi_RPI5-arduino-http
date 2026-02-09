# Service setup (systemd)

## Files in this folder
- arduino-web.service – runs the Flask web server on port 5000
- ngrok-arduino.service – starts ngrok tunnel to port 5000

## Install on Raspberry Pi
```bash
sudo cp "rpi5 services/arduino-web.service" /etc/systemd/system/
sudo cp "rpi5 services/ngrok-arduino.service" /etc/systemd/system/
```

Edit ngrok token:
```bash
sudo nano /etc/systemd/system/ngrok-arduino.service
# set Environment=NGROK_AUTHTOKEN=<your_token>
```

Enable and start:
```bash
sudo systemctl daemon-reload
sudo systemctl enable arduino-web ngrok-arduino
sudo systemctl start arduino-web ngrok-arduino
```

Check status and get public URL:
```bash
sudo systemctl status arduino-web ngrok-arduino
journalctl -u ngrok-arduino -f  # shows the https URL
```

Stop/disable:
```bash
sudo systemctl stop arduino-web ngrok-arduino
sudo systemctl disable arduino-web ngrok-arduino
```
