const WebSocket = require('websocket').client;
const os = require('os');
const si = require('systeminformation');
const client = new WebSocket();

// Replace with your WebSocket server URL
const serverUrl = 'ws://192.168.0.103/neko0x6a';

// JSON object to send
const message = {
    establishConnection: true
};

client.on('connectFailed', (error) => {
    console.log('Connection failed: ' + error.toString());
});

client.on('connect', (connection) => {
    console.log('Connected to server');

    connection.on('error', (error) => {
        console.log('Connection Error: ' + error.toString());
    });

    connection.on('close', () => {
        console.log('Connection closed');
    });

    connection.on('message', (message) => {
        if (message.type === 'utf8') {
            console.log('Received: ' + message.utf8Data);
        }
    });

    connection.sendUTF(JSON.stringify(message));

    setInterval(() => {
        const totalMemory = os.totalmem();
        const freeMemory = os.freemem();
        const usedMemory = totalMemory - freeMemory;
        const memoryUsage = Math.round((usedMemory / totalMemory) * 100);

        si.currentLoad((d) => {
            connection.sendUTF(JSON.stringify({
                updateUsage: true,
                cpuUsage: Math.round(d.currentLoad),
                memoryUsage: memoryUsage
            }));
            console.log(Math.round(d.currentLoad), memoryUsage)
        });

    }, 1000);
});

client.connect(serverUrl);
