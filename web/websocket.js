import { WebSocketServer } from 'ws';
import { readFileSync } from 'fs';

const PORT = 50003;

const wss = new WebSocketServer({ port: PORT });

wss.on('connection', function connection(ws) {
	console.log('New client connected');

	// 센서 데이터를 주기적으로 읽어서 전송
	const interval = setInterval(() => {
		try {
			const temperature = readFileSync('/tmp/temperature', 'utf8');
			const brightness = readFileSync('/tmp/brightness', 'utf8');
			const moisture = readFileSync('/tmp/moisture', 'utf8');

			ws.send(
				JSON.stringify({
					temperature: parseFloat(temperature),
					brightness: parseInt(brightness),
					moisture: parseFloat(moisture)
				})
			);
		} catch (err) {
			console.error('Error reading sensor data:', err);
		}
	}, 500); // 2초마다 데이터 전송

	ws.on('close', () => {
		clearInterval(interval);
		console.log('Client disconnected');
	});
});

console.log('WebSocket server running on port', PORT);
