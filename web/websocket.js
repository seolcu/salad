import { WebSocketServer } from 'ws';
import { readFileSync } from 'fs';
import { fileURLToPath } from 'url';
import { dirname } from 'path';

// ES 모듈에서 __dirname 사용하기 위한 설정
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// WebSocket 서버 생성 (포트 50003)
const wss = new WebSocketServer({ port: 50003 });

// 센서 데이터를 읽어오는 함수
function readSensorData() {
	try {
		const temperature = readFileSync('/tmp/temperature', 'utf8').trim();
		const soilmoisture = readFileSync('/tmp/soilmoisture', 'utf8').trim();
		const brightness = readFileSync('/tmp/brightness', 'utf8').trim();

		return {
			temperature: parseFloat(temperature),
			soilmoisture: parseFloat(soilmoisture),
			brightness: brightness
		};
	} catch (error) {
		console.error('Error reading sensor data:', error);
		return null;
	}
}

// 주기적으로 센서 데이터를 전송
function broadcastSensorData() {
	const data = readSensorData();
	if (data) {
		wss.clients.forEach((client) => {
			if (client.readyState === WebSocket.OPEN) {
				client.send(JSON.stringify(data));
			}
		});
	}
}

// 클라이언트 연결 시 처리
wss.on('connection', (ws) => {
	console.log('Client connected');

	// 초기 데이터 전송
	const data = readSensorData();
	if (data) {
		ws.send(JSON.stringify(data));
	}

	ws.on('close', () => {
		console.log('Client disconnected');
	});
});

// 1초마다 센서 데이터 브로드캐스트
setInterval(broadcastSensorData, 1000);

console.log('WebSocket server running on port 50003');
