<script lang="ts">
	import { onMount, onDestroy } from 'svelte';

	interface SensorData {
		temperature: number;
		soilmoisture: number;
		brightness: String;
	}

	let socket: WebSocket;
	let temperature: number | null = null;
	let soilmoisture: number | null = null;
	let brightness: String | null = null;
	let connectionStatus: string = 'Connecting...';

	// WebSocket 연결 설정
	function setupWebSocket(): void {
		socket = new WebSocket('ws://localhost:50003');

		socket.onopen = () => {
			connectionStatus = 'Connected';
		};

		socket.onmessage = (event: MessageEvent) => {
			const data: SensorData = JSON.parse(event.data);
			temperature = data.temperature;
			soilmoisture = data.soilmoisture;
			brightness = data.brightness;
		};

		socket.onclose = () => {
			connectionStatus = 'Disconnected';
			// 3초 후 재연결 시도
			setTimeout(setupWebSocket, 3000);
		};

		socket.onerror = (error: Event) => {
			connectionStatus = 'Error: ' + (error as ErrorEvent).message;
		};
	}

	onMount(() => {
		setupWebSocket();
	});

	onDestroy(() => {
		if (socket) {
			socket.close();
		}
	});
</script>

<div class="sensor-dashboard">
	<h2>Plant Sensor Dashboard</h2>
	<p class="connection-status" class:disconnected={connectionStatus !== 'Connected'}>
		Status: {connectionStatus}
	</p>

	<div class="sensor-grid">
		<div class="sensor-card">
			<h3>Temperature</h3>
			<p class="value">{temperature !== null ? `${temperature.toFixed(1)}°C` : '---'}</p>
		</div>

		<div class="sensor-card">
			<h3>Soil Moisture</h3>
			<p class="value">{soilmoisture !== null ? `${soilmoisture.toFixed(1)}%` : '---'}</p>
		</div>

		<div class="sensor-card">
			<h3>Brightness</h3>
			<p class="value">{brightness !== null ? brightness : '---'}</p>
		</div>
	</div>
</div>

<style>
	.sensor-dashboard {
		padding: 20px;
		max-width: 800px;
		margin: 0 auto;
	}

	.connection-status {
		color: #2ecc71;
		margin-bottom: 20px;
	}

	.connection-status.disconnected {
		color: #e74c3c;
	}

	.sensor-grid {
		display: grid;
		grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
		gap: 20px;
	}

	.sensor-card {
		background: #f8f9fa;
		border-radius: 8px;
		padding: 20px;
		text-align: center;
		box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
	}

	.sensor-card h3 {
		margin: 0 0 10px 0;
		color: #2c3e50;
	}

	.value {
		font-size: 24px;
		font-weight: bold;
		color: #34495e;
		margin: 0;
	}
</style>
