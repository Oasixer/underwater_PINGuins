<script lang="ts">
    import { onMount } from 'svelte';
    import Map from './Map.svelte';
    import Readouts from './Readouts.svelte';
    import type { Node, NodeDataDisplay } from "./node_data";
    import { rov_positions_pac } from "./node_data";
    import Video from './Video.svelte';

    let MEGA_TEMP_DONT_EVER_USE_SERVER_AAAAAAAAAAAAAAA = true;

    let counter: number = 0;

    let video_time_sec: number = 0;

    let node_data: NodeDataDisplay = {
        nodes: [],
        updated: new Date(),
    };
    
    let replay_pac = false;
    let replay_tick: number = 0;
    let replay_ping: number = 0;
    // let replay_started: Date;
    //
    let last_replay_video_time = 0;

    let replay_data: NodeDataDisplay;
    // const REPLAY_TICK_DURATION_MS = 10000;
    const REPLAY_TICK_DURATION_MS = 1320;
    const REPLAY_PING_DURATION_MS = REPLAY_TICK_DURATION_MS/4;

    function initialize_replay(){
      replay_tick = 0;
      replay_ping = 0;
      // replay_started = new Date();
      last_replay_video_time = 0;
      replay_data = {
        nodes: [
          {
            name:"SKPR",desc:"Skipper: ROV",idx:0,mac:[4,233,229,20,15,20],mac_str:"14",is_connected:true,
            coords:{"x":3.73,"y":13.05,"z":0}, last_ping: new Date(0),
          },
          {
            name:"RICO", desc:"Rico: Stationary 1",idx:1,mac:[4,233,229,20,14,241],mac_str:"f1",is_connected:true,
            coords:{"x":17.54,"y":13.445,"z":0}, last_ping: new Date(0),
          },
          {
            name:"KWSK",desc:"Kowalski: Stationary 2",idx:2,mac:[4,233,229,20,63,65],mac_str:"41",is_connected:true,
            coords:{"x":13.63,"y":6.515,"z":0}, last_ping: new Date(0),
          },
          {
            name:"PRVT",desc:"Private: Stationary 3",idx:3,mac:[4,233,229,20,63,30],mac_str:"1e",is_connected:true,
            coords:{"x":1.71,"y":13.405,"z":0}, last_ping: new Date(0),
          }
        ]
        ,"updated": new Date(),
      };
      replay_pac = true;
    }

    function updateReplayData(){
        // return '{"nodes":[{"name":"SKPR","desc":"Skipper: ROV","idx":0,"mac":[4,233,229,20,15,20],"mac_str":"14","is_connected":false,"coords":{"x":1,"y":1,"z":0}},{"name":"RICO", "desc":"Rico: Stationary 1","idx":1,"mac":[4,233,229,20,14,241],"mac_str":"f1","is_connected":false,"coords":{"x":6,"y":6,"z":0}},{"name":"KWSK","desc":"Kowalski: Stationary 2","idx":2,"mac":[4,233,229,20,63,65],"mac_str":"41","is_connected":false,"coords":{"x":5,"y":1,"z":0}},{"name":"PRVT","desc":"Private: Stationary 3","idx":3,"mac":[4,233,229,20,63,30],"mac_str":"1e","is_connected":false,"coords":{"x":1,"y":7,"z":0}}],"updated":1678710317637}';
        // return '{"nodes":[{"name":"SKPR","desc":"Skipper: ROV","idx":0,"mac":[4,233,229,20,15,20],"mac_str":"14","is_connected":false,"coords":{"x":0.5,"y":0.5,"z":0}},{"name":"RICO", "desc":"Rico: Stationary 1","idx":1,"mac":[4,233,229,20,14,241],"mac_str":"f1","is_connected":false,"coords":{"x":0.3,"y":0.3,"z":0}},{"name":"KWSK","desc":"Kowalski: Stationary 2","idx":2,"mac":[4,233,229,20,63,65],"mac_str":"41","is_connected":false,"coords":{"x":-0.3,"y":-0.3,"z":0}},{"name":"PRVT","desc":"Private: Stationary 3","idx":3,"mac":[4,233,229,20,63,30],"mac_str":"1e","is_connected":false,"coords":{"x":-0.6,"y":0,"z":0}}],"updated":1678710317637}';
      //{"nodes":[{"name":"SKPR","desc":"Skipper: ROV","idx":0,"mac":[4,233,229,20,15,20],"mac_str":"14","is_connected":false,"coords":{"x":0.5,"y":0.5,"z":0}, last_ping: 0},{"name":"RICO", "desc":"Rico: Stationary 1","idx":1,"mac":[4,233,229,20,14,241],"mac_str":"f1","is_connected":false,"coords":{"x":0.3,"y":0.3,"z":0}, last_ping: 0},{"name":"KWSK","desc":"Kowalski: Stationary 2","idx":2,"mac":[4,233,229,20,63,65],"mac_str":"41","is_connected":false,"coords":{"x":-0.3,"y":-0.3,"z":0}, last_ping: 0},{"name":"PRVT","desc":"Private: Stationary 3","idx":3,"mac":[4,233,229,20,63,30],"mac_str":"1e","is_connected":false,"coords":{"x":-0.6,"y":0,"z":0}, last_ping: 0}],"updated":1678710317637}
      let cur_time: Date = new Date();

      // let elapsedMs = cur_time.getTime()+1 - replay_started.getTime();
      if (Math.abs(video_time_sec - last_replay_video_time)*1000 > 500){
        replay_tick = Math.round(video_time_sec*1000 / REPLAY_TICK_DURATION_MS);
        replay_ping = 0;
      }
      last_replay_video_time = video_time_sec;
      let elapsedMs = Math.floor(video_time_sec * 1000);



      console.log("tick: "+replay_tick+ "nodes[0].coords.x: "+replay_data.nodes[0].coords.x);
      let elapsedMsInTick = elapsedMs - replay_tick * REPLAY_TICK_DURATION_MS;
      // console.log(`started: ${replay_started}`);
      console.log(`cur_time: ${cur_time} elapsedMs: ${elapsedMs}, elapsedMsInTick: ${elapsedMsInTick}`);
      if (elapsedMsInTick > REPLAY_TICK_DURATION_MS){
        replay_tick++;
        if (replay_tick >= rov_positions_pac.length){
          initialize_replay();
          return;
        }
        elapsedMsInTick = elapsedMs - replay_tick * REPLAY_TICK_DURATION_MS;
        replay_ping = 0;
        replay_data.nodes[replay_ping+1].last_ping = cur_time;
        // todo check for ending?
      }
      let elapsedMsInPing = elapsedMsInTick - replay_ping * REPLAY_PING_DURATION_MS;
      while (elapsedMsInPing > REPLAY_PING_DURATION_MS){
        replay_ping++;
        replay_data.nodes[replay_ping+1].last_ping = cur_time;
        elapsedMsInPing = elapsedMsInTick - replay_ping * REPLAY_PING_DURATION_MS;
        if (replay_ping == 2){
          replay_data.nodes[0].coords = rov_positions_pac[replay_tick];
        }
      }
      replay_data.updated = cur_time;
      // if (elapsedMsInTick ){}
      // return base_data;
    }

    function parseNodeDataDisplay(jsonData: any): NodeDataDisplay {
      // console.log(typeof jsonData.updated);
      // console.log(jsonData.updated);
      jsonData.updated = new Date(jsonData.updated);
      // console.log("pls...")
      // console.log(new Date(jsonData.updated));
      // console.log(jsonData.updated.toLocaleTimeString());
      const nodes = jsonData as NodeDataDisplay;
      nodes.updated = new Date(nodes.updated);
      for (let i=0; i<4; i++){
          nodes.nodes[i].last_ping = new Date(nodes.nodes[i].last_ping)
      }
      return nodes;
    }
    
    async function update(): Promise<NodeDataDisplay>{
      // return 1;
      if (replay_pac){
        console.log("updating replay data.");
        updateReplayData();
        return replay_data;
          // return parseNodeDataDisplay(exampleData(), replay_pac);
      } else {
        if (!MEGA_TEMP_DONT_EVER_USE_SERVER_AAAAAAAAAAAAAAA){
          const response = await fetch('/getNodeData'); // <-------------- server req for data!!
          const data = await response.json();
          return parseNodeDataDisplay(data);
        }
        else{ // TODO TEMP GET RID OF THIS BS!!!
          return node_data;
        }
      }
    }

    $: seconds = node_data.updated.getSeconds();

  let mounted = false;
  let updatedStr = "";

  let mouseX: number;
  let mouseY: number;
//   let map;
  onMount(() => {
    const intervalId = setInterval(async () => {
        node_data = await update();
        console.log("updated: " + node_data.updated);
        console.log("updated: " + node_data.updated.toLocaleTimeString());
        updatedStr = node_data.updated.toLocaleTimeString();
        counter = counter + 1;
        mounted = true;
    }, REPLAY_PING_DURATION_MS/4);

//     const rect = document.querySelector('Map') as any;
//     const rect = map;
//     rect.addEventListener('mousemove', updateIconPosition);
    return () => clearInterval(intervalId);
  });
</script>

<div class='outer'>
    <div class='rect'>
        <div>
            <img src="logo_text_new.png" alt="" style="width: 450px; height: auto; margin-left: 40px;"/>
            {#if replay_pac}
              <Video bind:video_time_sec/> 
            {/if}
            {#if !replay_pac}
              <div style="height: 100%;">
                  <button id="startReplay" on:click={()=>{initialize_replay()}}>Show Demo</button>
                  <Readouts bind:node_data bind:counter bind:mouseX bind:mouseY/>
              </div>
            {/if}
        </div>
        <div style="margin-left: auto; margin-right: auto; padding-top: {replay_pac?'80px':'0px'}">
            <Map {replay_pac} bind:node_data bind:counter bind:mouseX bind:mouseY/>
        </div>
    </div>
</div>
<style lang="scss">
	button#startReplay{
    cursor: pointer;
    border: none;
    background: none;
    outline: none;
    font-weight: 400;
    font-family: "Helvetica", serif;
    font-size:25px;
    text-transform: none;
		width: 230px;
		height: 50px;
		//position: relative;
		/* margin-right: auto; */
		/* margin-left: auto; */
		margin: auto;
    margin-left: 100px;
  padding-bottom: 10px;
  margin-bottom: 20px;
    /* color: #000000; */
    color: #ffffff;
    background-color: #86A8D6;
    border-radius: 5px;
    padding: 9px;
		z-index: 999;
    opacity: 100%;
    /* margin: auto; */
    font-family: "Roboto", "Helvetica", "Arial", sans-serif;
    font-weight: 400;
    letter-spacing: 1px;
    /* font-family: "Open Sans", sans-serif; */
    /* font-family: "Helvetica", serif; */
    padding: 10px 9px;
    border-radius: 5px;
		box-shadow: 0 16px 38px -12px rgb(0 0 0 / 56%), 0 4px 25px 0px rgb(0 0 0 / 12%), 0 8px 10px -5px rgb(0 0 0 / 20%);
    /* margin: 35vh auto 0 auto; */
    /* left: 40%; */
		/* position: absolute; */
		z-index: 100;
   /* -webkit-transform: translateX(-40%); */
   /* -moz-transform: translateX(-40%); */
   /* transform: translateX(-40%), translateY(-40%); */
    /* top: 40%; */
   /* -webkit-transform: translateY(-40%); */
   /* -moz-transform: translateY(-40%); */
   /* transform: translateY(-40%); */
	}
  button#learnMore:active{
    background: none;
    outline: none;
  }
  
  button#learnMore:hover{
    /* color: #f3f5f4; */
		background-color: #759ACE;
  }

  button#learnMore.selected:focus{
    outline: none;
  }
    div.outer {
        width: 100%;
        max-width: 100vw;
        overflow: clip;
        height: 100vh;
        max-height: 100vh;
        margin-top: 0;
        min-height: 100%;
        // background-color: #031767;
        div.rect {
            // take up 100% of the width and height of the parent
            width: 100%;
            height: 100%;
            // margin: 50px;
            padding: 30px;
            border: 2px solid black;
            background-color: #031767;
            display: flex;
            flex-flow: row nowrap;
        }
    }
</style>
