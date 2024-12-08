<script lang="ts">
	import { onMount, onDestroy } from 'svelte';

	let temperature: number = 0;
	let light: boolean = true;
	let moisture: number = 0;
	let ws: WebSocket | null = null;

	onMount(() => {
		// WebSocket 연결 설정
		ws = new WebSocket('ws://localhost:50003');

		ws.onmessage = (event: MessageEvent) => {
			const data = JSON.parse(event.data);
			temperature = data.temperature;
			light = Boolean(data.light);
			moisture = data.moisture;
		};

		ws.onerror = (error: Event) => {
			console.error('WebSocket error:', error);
		};
	});

	onDestroy(() => {
		if (ws) ws.close();
	});
</script>

<main class="container">
	<h1>실시간 센서 데이터</h1>

	<div class="sensor-data">
		<div class="card">
			<h2>온도</h2>
			<p>{temperature}°C</p>
		</div>

		<div class="card">
			<h2>밝기</h2>
			<p>{light}</p>
		</div>

		<div class="card">
			<h2>토양습도</h2>
			<p>{moisture}%</p>
		</div>
	</div>
</main>
