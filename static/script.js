async function fetchStatus() {
    try {
        const response = await fetch('/api/nodemcu/status');
        if (response.ok) {
            const text = await response.text();
            document.getElementById('mcu-status').innerHTML = text;
        } else {
            document.getElementById('mcu-status').innerHTML = 'Error fetching status';
        }
    } catch (error) {
        document.getElementById('mcu-status').innerHTML = 'Connection error';
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

async function toggleChannel(channel) {
    try {
        const response = await fetch(`/api/nodemcu/toggle/${channel}`, { method: 'POST' });
        if (response.ok) {
            await updateRelayStatus();
        } else {
            alert('Failed to toggle channel');
        }
    } catch (error) {
        alert('Connection error');
    }
}

// Fetch initial status and set interval for updates
fetchStatus();
updateRelayStatus();
setInterval(updateRelayStatus, 5000);