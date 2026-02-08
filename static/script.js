async function fetchD1Status() {
    try {
        const response = await fetch('/api/d1/status');
        if (response.ok) {
            const data = await response.json();
            document.getElementById('d1-status').innerHTML = JSON.stringify(data, null, 2);
        } else {
            document.getElementById('d1-status').innerHTML = 'Error fetching Arduino D1 status';
        }
    } catch (error) {
        document.getElementById('d1-status').innerHTML = 'Connection error';
    }
}

async function fetchNodeMCUStatus() {
    try {
        const response = await fetch('/api/nodemcu/status');
        if (response.ok) {
            const text = await response.text();
            document.getElementById('nodemcu-status').innerHTML = text;
        } else {
            document.getElementById('nodemcu-status').innerHTML = 'Error fetching NodeMCU status';
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

// Fetch initial statuses and set interval for updates
fetchD1Status();
fetchNodeMCUStatus();
setInterval(fetchD1Status, 5000);
setInterval(fetchNodeMCUStatus, 5000);