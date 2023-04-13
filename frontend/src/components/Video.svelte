<script>
	// These values are bound to properties of the video
	// let video_time_sec = 0;
	let duration;
	let paused = true;

	let showControls = true;
	let showControlsTimeout;
  
  export let video_time_sec;

	// Used to track time of last mouse down event
	let lastMouseDown;

	function handleMove(e) {
		// Make the controls visible, but fade out after
		// 2.5 seconds of inactivity
		clearTimeout(showControlsTimeout);
		showControlsTimeout = setTimeout(() => showControls = false, 2500);
		showControls = true;

		if (!duration) return; // video not loaded yet
		if (e.type !== 'touchmove' && !(e.buttons & 1)) return; // mouse not down

		const clientX = e.type === 'touchmove' ? e.touches[0].clientX : e.clientX;
		const { left, right } = this.getBoundingClientRect();
		video_time_sec = duration * (clientX - left) / (right - left);
	}

	// we can't rely on the built-in click event, because it fires
	// after a drag — we have to listen for clicks ourselves
	function handleMousedown(e) {
		lastMouseDown = new Date();
	}

	function handleMouseup(e) {
		if (new Date() - lastMouseDown < 300) {
			if (paused) e.target.play();
			else e.target.pause();
		}
	}

	function format(seconds) {
    // console.log("seconds: "+ seconds);
		if (isNaN(seconds)) return '...';

		const minutes = Math.floor(seconds / 60);
		seconds = Math.floor(seconds % 60);
		if (seconds < 10) seconds = '0' + seconds;

		return `${minutes}:${seconds}`;
	}
</script>

<!-- <h1>Caminandes: Llamigos</h1> -->
<!-- <p>From <a href="https://studio.blender.org/films">Blender Studio</a>. CC-BY</p> -->

<!--
		src=""//https://sveltejs.github.io/assets/caminandes-llamigos.mp4" -->
<div style="width: 95%">
<!-- </div>; border-radius: 5px; overflow: hidden;"> -->
	<video
		poster="thumb.png"
		src="pac_demo.mp4"
		on:mousemove={handleMove}
		on:touchmove|preventDefault={handleMove}
		on:mousedown={handleMousedown}
		on:mouseup={handleMouseup}
    on:ended={()=>{video_time_sec=0; paused=false;}}
		bind:currentTime={video_time_sec}
		bind:duration
		bind:paused>
		<track kind="captions">
	</video>

	<div class="controls" style="opacity: {duration && showControls ? 1 : 0}">
		<progress value="{(video_time_sec / duration) || 0}"/>

		<div class="info">
			<span class="time">{format(video_time_sec)}</span>
      <!--<span>click anywhere to {paused ? 'play' : 'pause'} / drag to seek</span>-->
			<span class="time">{format(duration)}</span>
		</div>
	</div>
</div>

<style>
	div {
		position: relative;
	}

	.controls {
		position: absolute;
		top: 0;
		width: 100%;
		transition: opacity 1s;
	}

	.info {
		display: flex;
		width: 100%;
		justify-content: space-between;
	}

	span {
		padding: 0.2em 0.5em;
		color: white;
		text-shadow: 0 0 8px black;
		font-size: 1.4em;
		opacity: 0.7;
	}

	.time {
		width: 3em;
	}

	.time:last-child { text-align: right }

	progress {
		display: block;
		width: 99%;
		height: 18px;
		-webkit-appearance: none;
		appearance: none;
    margin: 5px;
	}

	progress::-webkit-progress-bar {
		background-color: rgba(0,0,0,0.5);
    /* margin: 5px; */
    border-radius: 7px;
	}

	progress::-webkit-progress-value {
		/* background-color: rgba(255,255,255,0.6); */
		/* background-color: rgba(25,25,255,0.6); */
    background-color: #96b8e6;
    border: 1px solid white;
    border-radius: 7px;
    /* marg */
    /* border-bottom-left-radius:0; */
    /* margin-left: 5px; */
	}

	video {
		width: 100%;
    border-radius: 7px;
	}
</style>
