async function fetchD1Details() {
    try {
        const response = await fetch('/api/d1/status');
        if (response.ok) {
            const data = await response.json();
            document.getElementById('d1-status').innerHTML = JSON.stringify(data, null, 2);
        } else {
            document.getElementById('d1-status').innerHTML = 'Error fetching Arduino D1 details';
        }
    } catch (error) {
        document.getElementById('d1-status').innerHTML = 'Connection error';
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

// Fetch initial details and set interval for updates
fetchD1Details();
fetchNodeMCUDetails();
setInterval(fetchD1Details, 5000);
setInterval(fetchNodeMCUDetails, 5000);