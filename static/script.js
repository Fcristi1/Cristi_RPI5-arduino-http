async function fetchD1Details() {
    try {
        const response = await fetch('http://192.168.0.37:8080/status'); // Correct endpoint
        if (response.ok) {
            const data = await response.json();

            // Update specific fields
            document.getElementById('arduino1-status').innerText = 'Connected';
            document.getElementById('arduino1-led').innerText = data.led || 'Unknown';
            document.getElementById('arduino1-temp').innerText = data.temperature || '-';
            document.getElementById('arduino1-humidity').innerText = data.humidity || '-';
        } else {
            document.getElementById('arduino1-status').innerText = 'Error fetching status';
        }
    } catch (error) {
        document.getElementById('arduino1-status').innerText = 'Connection error';
    }
}

async function fetchNodeMCUDetails() {
    try {
        const response = await fetch('/api/nodemcu/status');
        if (response.ok) {
            const text = await response.text();
            document.getElementById('nodemcu-status').innerHTML = text;

            // Update relay statuses based on response
            const channel1 = text.includes('Channel 1: ON') ? 'ON' : 'OFF';
            const channel2 = text.includes('Channel 2: ON') ? 'ON' : 'OFF';
            document.getElementById('relay1-status').innerText = channel1;
            document.getElementById('relay2-status').innerText = channel2;
        } else {
            document.getElementById('nodemcu-status').innerHTML = 'Error fetching NodeMCU details';
        }
    } catch (error) {
        document.getElementById('nodemcu-status').innerHTML = 'Connection error';
    }
}

async function updateRelayStatus() {
    try {
        const response = await fetch('/api/nodemcu/status');
        if (response.ok) {
            const text = await response.text();
            const channel1 = text.includes('Channel 1: ON') ? 'ON' : 'OFF';
            const channel2 = text.includes('Channel 2: ON') ? 'ON' : 'OFF';

            document.getElementById('channel1-status').innerText = channel1;
            document.getElementById('channel2-status').innerText = channel2;
        } else {
            document.getElementById('channel1-status').innerText = 'Error';
            document.getElementById('channel2-status').innerText = 'Error';
        }
    } catch (error) {
        document.getElementById('channel1-status').innerText = 'Connection error';
        document.getElementById('channel2-status').innerText = 'Connection error';
    }
}

async function toggleNodeMCULed() {
    try {
        const response = await fetch('/api/nodemcu/led/toggle', { method: 'POST' });
        if (!response.ok) {
            alert('Failed to toggle NodeMCU LED');
        }
    } catch (error) {
        alert('Connection error');
    }
}

async function toggleRelay(channel) {
    try {
        const response = await fetch(`/api/nodemcu/toggle/${channel}`, { method: 'POST' });
        if (response.ok) {
            await fetchNodeMCUDetails();
        } else {
            alert(`Failed to toggle Relay Channel ${channel}`);
        }
    } catch (error) {
        alert('Connection error');
    }
}

function toggleArduino1LED() {
    console.log("Toggling Arduino 1 LED");
    const message = new Paho.MQTT.Message("TOGGLE_LED");
    message.destinationName = "arduino1/command";
    mqttClient.send(message);
}

function toggleArduino3LED() {
    console.log("Toggling Arduino 3 LED");
    const message = new Paho.MQTT.Message("TOGGLE_LED");
    message.destinationName = "arduino3/command";
    mqttClient.send(message);
}

// Connect to MQTT broker on page load
connectMQTT();

// Fetch initial details and set interval for updates
fetchD1Details();
fetchNodeMCUDetails();
setInterval(fetchD1Details, 5000);
setInterval(fetchNodeMCUDetails, 5000);

async function fetchSensorData() {
    try {
        const response = await fetch('http://192.168.0.37:8080/sensor'); // Fetch sensor data
        if (response.ok) {
            const data = await response.json();

            // Update temperature and humidity fields
            document.getElementById('arduino1-temp').innerText = data.temperature ? `${data.temperature} °C` : '-';
            document.getElementById('arduino1-humidity').innerText = data.humidity ? `${data.humidity} %` : '-';
        } else {
            document.getElementById('arduino1-temp').innerText = 'Error';
            document.getElementById('arduino1-humidity').innerText = 'Error';
        }
    } catch (error) {
        document.getElementById('arduino1-temp').innerText = 'Connection error';
        document.getElementById('arduino1-humidity').innerText = 'Connection error';
    }
}

async function fetchStatus() {
    try {
        const response = await fetch('http://192.168.0.37:8080/status'); // Fetch status data
        if (response.ok) {
            const data = await response.json();

            // Update LED status
            document.getElementById('arduino1-led').innerText = data.led || 'Unknown';
        } else {
            document.getElementById('arduino1-led').innerText = 'Error';
        }
    } catch (error) {
        document.getElementById('arduino1-led').innerText = 'Connection error';
    }
}

async function fetchTemperature() {
    try {
        console.log("Fetching temperature..."); // Debug log
        const response = await fetch('/api/temperature'); // Fetch temperature data
        if (response.ok) {
            const data = await response.json();
            console.log("Temperature data received:", data); // Debug log
            document.getElementById('arduino1-temp').innerText = data.temperature ? `${data.temperature} °C` : '-';
        } else {
            console.error("Error fetching temperature: HTTP", response.status); // Debug log
            document.getElementById('arduino1-temp').innerText = 'Error';
        }
    } catch (error) {
        console.error("Connection error while fetching temperature:", error); // Debug log
        document.getElementById('arduino1-temp').innerText = 'Connection error';
    }
}

async function fetchArduino1SensorData() {
    try {
        const response = await fetch('/api/arduino1/sensor');
        if (response.ok) {
            const data = await response.json();
            document.getElementById('arduino1-status').innerText = 'Connected';
            document.getElementById('arduino1-temp').innerText = data.temperature || '-';
            document.getElementById('arduino1-humidity').innerText = data.humidity || '-';
        } else {
            document.getElementById('arduino1-status').innerText = 'Disconnected';
        }
    } catch (error) {
        document.getElementById('arduino1-status').innerText = 'Disconnected';
    }
}

async function toggleArduino1Pin(pin, state) {
    try {
        await fetch('/api/arduino1/control', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ pin, state })
        });
    } catch (error) {
        console.error('Failed to toggle Arduino 1 pin:', error);
    }
}

async function toggleArduino3Pin(pin, state) {
    try {
        await fetch('/api/arduino3/control', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ pin, state })
        });
    } catch (error) {
        console.error('Failed to toggle Arduino 3 pin:', error);
    }
}

// Call fetchSensorData and fetchStatus periodically to update the values
setInterval(fetchSensorData, 5000); // Update sensor data every 5 seconds
setInterval(fetchStatus, 5000); // Update status every 5 seconds
setInterval(fetchTemperature, 5000); // Update temperature every 5 seconds
setInterval(fetchArduino1SensorData, 5000); // Update Arduino 1 sensor data every 5 seconds

const mqttBroker = "ws://192.168.0.100:9001"; // Replace with your MQTT broker WebSocket URL
const mqttClient = new Paho.MQTT.Client(mqttBroker, "web_client");

mqttClient.onConnectionLost = (responseObject) => {
    console.error("Connection lost:", responseObject.errorMessage);
};

mqttClient.onMessageArrived = (message) => {
    console.log("Message arrived:", message.payloadString);
    const topic = message.destinationName;
    const payload = JSON.parse(message.payloadString);

    if (topic === "arduino1/sensor") {
        console.log("Updating Arduino 1 sensor data");
        document.getElementById('arduino1-status').innerText = 'Connected';
        document.getElementById('arduino1-temp').innerText = payload.temperature || '-';
        document.getElementById('arduino1-humidity').innerText = payload.humidity || '-';
    } else if (topic === "arduino3/status") {
        console.log("Updating Arduino 3 status");
        document.getElementById('arduino3-status').innerText = payload.connected ? 'Connected' : 'Disconnected';
        document.getElementById('arduino3-relay0').innerText = payload.relay0 || 'OFF';
        document.getElementById('arduino3-relay1').innerText = payload.relay1 || 'OFF';
    }
};

function connectMQTT() {
    mqttClient.connect({
        onSuccess: () => {
            console.log("Connected to MQTT broker");
            mqttClient.subscribe("arduino1/sensor");
            mqttClient.subscribe("arduino3/status");
        },
        onFailure: (error) => {
            console.error("Failed to connect to MQTT broker:", error);
        }
    });
}

// Connect to MQTT broker on page load
connectMQTT();