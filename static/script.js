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
    fetch('http://192.168.0.37:8080/led/toggle')
        .then(response => response.json())
        .then(data => {
            document.getElementById('arduino1-led').innerText = data.status.includes('ON') ? 'ON' : 'OFF';
        })
        .catch(() => {
            document.getElementById('arduino1-led').innerText = 'Error';
        });
}

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

// Call fetchSensorData and fetchStatus periodically to update the values
setInterval(fetchSensorData, 5000); // Update sensor data every 5 seconds
setInterval(fetchStatus, 5000); // Update status every 5 seconds
setInterval(fetchTemperature, 5000); // Update temperature every 5 seconds