console.log('[INIT] Script loaded');

async function fetchArduino1Status() {
    try {
        const response = await fetch('/api/arduino1/status');
        console.log('[DEBUG] Fetching Arduino 1 status. Response status:', response.status);
        
        if (response.ok) {
            const data = await response.json();
            console.log('[DEBUG] Arduino 1 data:', data);
            document.getElementById('arduino1-status').innerText = 'Connected';
            document.getElementById('arduino1-led').innerText = data.led || 'OFF';
            document.getElementById('arduino1-temp').innerText = data.temperature || '-';
            document.getElementById('arduino1-humidity').innerText = data.humidity || '-';
        } else {
            document.getElementById('arduino1-status').innerText = 'Disconnected';
            console.warn('[WARN] Arduino 1 status returned:', response.status);
        }
    } catch (error) {
        document.getElementById('arduino1-status').innerText = 'Disconnected';
        console.error('[ERROR] Error fetching Arduino 1 status:', error);
    }
}

async function fetchArduino3Status() {
    try {
        const response = await fetch('/api/arduino3/status');
        if (response.ok) {
            const data = await response.json();
            document.getElementById('arduino3-status').innerText = 'Connected';
            document.getElementById('arduino3-led').innerText = data.builtin_led || 'OFF';
            document.getElementById('arduino3-relay0').innerText = data.relay_channel_1 || 'OFF';
            document.getElementById('arduino3-relay1').innerText = data.relay_channel_2 || 'OFF';
        } else {
            document.getElementById('arduino3-status').innerText = 'Disconnected';
        }
    } catch (error) {
        document.getElementById('arduino3-status').innerText = 'Disconnected';
        console.error('Error fetching Arduino 3 status:', error);
    }
}

async function toggleArduino1LED() {
    console.log('[DEBUG] toggleArduino1LED called');
    try {
        const response = await fetch('/api/arduino1/led/toggle', { method: 'POST' });
        console.log('[DEBUG] Response status:', response.status);
        
        if (response.ok) {
            const data = await response.json();
            console.log('[DEBUG] Toggle response:', data);
            await fetchArduino1Status();
        } else {
            const error = await response.text();
            console.error('[ERROR] Failed to toggle LED. Status:', response.status, 'Error:', error);
            alert('Failed to toggle Arduino 1 LED: ' + response.status);
        }
    } catch (error) {
        console.error('[ERROR] Connection error:', error);
        alert('Connection error: ' + error.message);
    }
}

async function toggleArduino3LED() {
    try {
        const response = await fetch('/api/arduino3/led/toggle', { method: 'POST' });
        if (response.ok) {
            await fetchArduino3Status();
        } else {
            alert('Failed to toggle Arduino 3 LED');
        }
    } catch (error) {
        alert('Connection error');
        console.error('Error toggling Arduino 3 LED:', error);
    }
}

// Fetch initial status and set up periodic updates
fetchArduino1Status();
fetchArduino3Status();
setInterval(fetchArduino1Status, 5000);
setInterval(fetchArduino3Status, 5000);