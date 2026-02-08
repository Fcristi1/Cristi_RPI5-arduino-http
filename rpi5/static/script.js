// Global state
let currentStatus = {
    led: 'OFF',
    builtin_led: 'OFF',
    button: 'RELEASED',
    ip: 'Unknown',
    connected: false,
    last_update: null
};

// Global state for Arduino 3
let arduino3Status = {
    connected: false,
    ip: 'Unknown',
    last_update: null
};

// Initialize on page load
document.addEventListener('DOMContentLoaded', function() {
    console.log('Initializing Arduino D1 Control Interface...');
    
    // Initial status update
    updateStatus();
    
    // Update status every 2 seconds
    setInterval(updateStatus, 2000);
    
    // Load Arduino IP from localStorage if exists
    const savedIp = localStorage.getItem('arduinoIp');
    if (savedIp) {
        document.getElementById('arduinoIpInput').value = savedIp;
    }
});

/**
 * Update current status from server
 */
function updateStatus() {
    fetch('/api/status')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            currentStatus = data;
            updateUI();
        })
        .catch(error => {
            console.error('Error fetching status:', error);
            updateConnectionStatus(false);
        });
}

/**
 * Update UI elements with current status
 */
function updateUI() {
    // Update connection status
    const connected = currentStatus.connected;
    updateConnectionStatus(connected);
    
    // Update LED display
    const ledIndicator = document.getElementById('ledIndicator');
    const ledStatus = document.getElementById('ledStatus');
    
    if (currentStatus.led === 'ON') {
        ledIndicator.className = 'led-on';
        ledStatus.textContent = 'LED: ON';
    } else {
        ledIndicator.className = 'led-off';
        ledStatus.textContent = 'LED: OFF';
    }
    
    // Update button display
    const buttonIndicator = document.getElementById('buttonIndicator');
    const buttonStatus = document.getElementById('buttonStatus');
    
    if (currentStatus.button === 'PRESSED') {
        buttonIndicator.className = 'button-pressed';
        buttonStatus.textContent = 'Button: PRESSED';
    } else {
        buttonIndicator.className = 'button-released';
        buttonStatus.textContent = 'Button: RELEASED';
    }

    // Update built-in LED display
    const builtinLedIndicator = document.getElementById('builtinLedIndicator');
    const builtinLedStatus = document.getElementById('builtinLedStatus');

    if (currentStatus.builtin_led === 'ON') {
        builtinLedIndicator.className = 'led-on';
        builtinLedStatus.textContent = 'Built-in LED: ON';
    } else {
        builtinLedIndicator.className = 'led-off';
        builtinLedStatus.textContent = 'Built-in LED: OFF';
    }
    
    // Update Arduino IP display
    document.getElementById('arduinoIp').textContent = currentStatus.ip || 'Unknown';
    
    // Update last update time
    if (currentStatus.last_update) {
        document.getElementById('lastUpdate').textContent = currentStatus.last_update;
    }
}

/**
 * Update connection status indicator
 */
function updateConnectionStatus(connected) {
    const dot = document.getElementById('connectionDot');
    const statusText = document.getElementById('statusText');
    const statusInfo = document.getElementById('statusInfo');
    const arduinoStatus = document.getElementById('arduinoStatus');
    
    if (connected) {
        dot.className = 'status-dot connected';
        statusText.textContent = 'Connected';
        statusInfo.textContent = 'Arduino D1 is online';
        arduinoStatus.innerHTML = 'Arduino: <span class="online">Online</span>';
    } else {
        dot.className = 'status-dot disconnected';
        statusText.textContent = 'Disconnected';
        statusInfo.textContent = 'Unable to connect to Arduino';
        arduinoStatus.innerHTML = 'Arduino: <span class="offline">Offline</span>';
    }
}

/**
 * LED Control - Turn ON
 */
function ledOn() {
    if (!currentStatus.connected) {
        showNotification('Arduino not connected', 'error');
        return;
    }
    
    fetch('/api/led/on', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            console.log('LED On:', data);
            updateStatus();
            showNotification('LED turned ON', 'success');
        })
        .catch(error => {
            console.error('Error:', error);
            showNotification('Failed to turn LED on', 'error');
        });
}

/**
 * LED Control - Turn OFF
 */
function ledOff() {
    if (!currentStatus.connected) {
        showNotification('Arduino not connected', 'error');
        return;
    }
    
    fetch('/api/led/off', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            console.log('LED Off:', data);
            updateStatus();
            showNotification('LED turned OFF', 'success');
        })
        .catch(error => {
            console.error('Error:', error);
            showNotification('Failed to turn LED off', 'error');
        });
}

/**
 * LED Control - Toggle
 */
function ledToggle() {
    if (!currentStatus.connected) {
        showNotification('Arduino not connected', 'error');
        return;
    }
    
    fetch('/api/led/toggle', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            console.log('LED Toggled:', data);
            updateStatus();
            showNotification(data.action, 'success');
        })
        .catch(error => {
            console.error('Error:', error);
            showNotification('Failed to toggle LED', 'error');
        });
}

/**
 * Built-in LED Control - Turn ON
 */
function builtinLedOn() {
    if (!currentStatus.connected) {
        showNotification('Arduino not connected', 'error');
        return;
    }

    fetch('/api/builtin/on', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            console.log('Built-in LED On:', data);
            updateStatus();
            showNotification('Built-in LED turned ON', 'success');
        })
        .catch(error => {
            console.error('Error:', error);
            showNotification('Failed to turn built-in LED on', 'error');
        });
}

/**
 * Built-in LED Control - Turn OFF
 */
function builtinLedOff() {
    if (!currentStatus.connected) {
        showNotification('Arduino not connected', 'error');
        return;
    }

    fetch('/api/builtin/off', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            console.log('Built-in LED Off:', data);
            updateStatus();
            showNotification('Built-in LED turned OFF', 'success');
        })
        .catch(error => {
            console.error('Error:', error);
            showNotification('Failed to turn built-in LED off', 'error');
        });
}

/**
 * Built-in LED Control - Toggle
 */
function builtinLedToggle() {
    if (!currentStatus.connected) {
        showNotification('Arduino not connected', 'error');
        return;
    }

    fetch('/api/builtin/toggle', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            console.log('Built-in LED Toggled:', data);
            updateStatus();
            showNotification(data.action || 'Built-in LED toggled', 'success');
        })
        .catch(error => {
            console.error('Error:', error);
            showNotification('Failed to toggle built-in LED', 'error');
        });
}

/**
 * Update Arduino IP address
 */
function updateArduinoIp() {
    const newIp = document.getElementById('arduinoIpInput').value.trim();
    
    if (!newIp) {
        showNotification('Please enter an IP address', 'error');
        return;
    }
    
    // Validate IP format (basic)
    const ipRegex = /^(\d{1,3}\.){3}\d{1,3}$/;
    if (!ipRegex.test(newIp)) {
        showNotification('Invalid IP address format', 'error');
        return;
    }
    
    // Send update to server
    fetch('/api/config', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ ip: newIp })
    })
        .then(response => response.json())
        .then(data => {
            console.log('Config updated:', data);
            
            if (data.status === 'success') {
                // Save to localStorage
                localStorage.setItem('arduinoIp', newIp);
                
                // Update status immediately
                updateStatus();
                showNotification(`Arduino IP updated to ${newIp}`, 'success');
            } else {
                showNotification(data.message || 'Failed to update configuration', 'error');
            }
        })
        .catch(error => {
            console.error('Error updating config:', error);
            showNotification('Failed to update configuration', 'error');
        });
}

/**
 * Fetch and update sensor data
 */
function updateSensorData() {
    fetch('/api/sensor')
        .then(response => {
            if (!response.ok) {
                throw new Error('Failed to fetch sensor data');
            }
            return response.json();
        })
        .then(data => {
            document.getElementById('temperature').textContent = data.temperature || '-';
            document.getElementById('humidity').textContent = data.humidity || '-';
        })
        .catch(error => {
            console.error('Error fetching sensor data:', error);
            document.getElementById('temperature').textContent = '-';
            document.getElementById('humidity').textContent = '-';
        });
}

// Update sensor data every 2 seconds
setInterval(updateSensorData, 2000);

/**
 * Update Arduino 3 status from server
 */
function updateArduino3Status() {
    fetch('/api/nodemcu/status')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            arduino3Status = data;
            updateArduino3UI();
        })
        .catch(error => {
            console.error('Error fetching Arduino 3 status:', error);
            document.getElementById('arduino3Connection').innerText = 'Disconnected';
            document.getElementById('arduino3Ip').innerText = '-';
            document.getElementById('arduino3LastUpdate').innerText = '-';
        });
}

/**
 * Update UI elements with Arduino 3 status
 */
function updateArduino3UI() {
    document.getElementById('arduino3Connection').innerText = arduino3Status.connected ? 'Connected' : 'Disconnected';
    document.getElementById('arduino3Ip').innerText = arduino3Status.ip || '-';
    document.getElementById('arduino3LastUpdate').innerText = arduino3Status.last_update || '-';
}

/**
 * Show notification (simple version)
 * In production, use a proper notification library
 */
function showNotification(message, type) {
    // Log to console for now
    if (type === 'error') {
        console.error('❌', message);
    } else if (type === 'success') {
        console.log('✓', message);
    } else {
        console.log('ℹ', message);
    }
    
    // Optional: Create a temporary alert at top of page
    const notification = document.createElement('div');
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        padding: 15px 20px;
        background-color: ${type === 'error' ? '#ef4444' : '#10b981'};
        color: white;
        border-radius: 8px;
        z-index: 1000;
        font-weight: 600;
        animation: slideIn 0.3s ease, slideOut 0.3s ease 2.7s;
    `;
    notification.textContent = message;
    
    // Add animations
    const style = document.createElement('style');
    style.textContent = `
        @keyframes slideIn {
            from {
                transform: translateX(400px);
                opacity: 0;
            }
            to {
                transform: translateX(0);
                opacity: 1;
            }
        }
        @keyframes slideOut {
            from {
                transform: translateX(0);
                opacity: 1;
            }
            to {
                transform: translateX(400px);
                opacity: 0;
            }
        }
    `;
    
    if (!document.querySelector('style[data-notifications]')) {
        style.setAttribute('data-notifications', 'true');
        document.head.appendChild(style);
    }
    
    document.body.appendChild(notification);
    
    // Remove notification after 3 seconds
    setTimeout(() => {
        notification.remove();
    }, 3000);
}

// Keyboard shortcuts
document.addEventListener('keydown', function(event) {
    // Ctrl/Cmd + 1: LED On
    if ((event.ctrlKey || event.metaKey) && event.key === '1') {
        event.preventDefault();
        ledOn();
    }
    
    // Ctrl/Cmd + 0: LED Off
    if ((event.ctrlKey || event.metaKey) && event.key === '0') {
        event.preventDefault();
        ledOff();
    }
    
    // Ctrl/Cmd + T: Toggle LED
    if ((event.ctrlKey || event.metaKey) && event.key === 't') {
        event.preventDefault();
        ledToggle();
    }
});
