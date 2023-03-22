<script lang="ts">
    import { onMount } from 'svelte';
    import Map from './Map.svelte';
    import Readouts from './Readouts.svelte';
    import type { Node, NodeDataDisplay } from "./node_data";

    let counter: number = 0;

    let node_data: NodeDataDisplay = {
        nodes: [],
        updated: new Date(),
    };
    function exampleData(){
        // return '{"nodes":[{"name":"SKPR","desc":"Skipper: ROV","idx":0,"mac":[4,233,229,20,15,20],"mac_str":"14","is_connected":false,"coords":{"x":1,"y":1,"z":0}},{"name":"RICO", "desc":"Rico: Stationary 1","idx":1,"mac":[4,233,229,20,14,241],"mac_str":"f1","is_connected":false,"coords":{"x":6,"y":6,"z":0}},{"name":"KWSK","desc":"Kowalski: Stationary 2","idx":2,"mac":[4,233,229,20,63,65],"mac_str":"41","is_connected":false,"coords":{"x":5,"y":1,"z":0}},{"name":"PRVT","desc":"Private: Stationary 3","idx":3,"mac":[4,233,229,20,63,30],"mac_str":"1e","is_connected":false,"coords":{"x":1,"y":7,"z":0}}],"updated":1678710317637}';
        return '{"nodes":[{"name":"SKPR","desc":"Skipper: ROV","idx":0,"mac":[4,233,229,20,15,20],"mac_str":"14","is_connected":false,"coords":{"x":0.5,"y":0.5,"z":0}},{"name":"RICO", "desc":"Rico: Stationary 1","idx":1,"mac":[4,233,229,20,14,241],"mac_str":"f1","is_connected":false,"coords":{"x":0.3,"y":0.3,"z":0}},{"name":"KWSK","desc":"Kowalski: Stationary 2","idx":2,"mac":[4,233,229,20,63,65],"mac_str":"41","is_connected":false,"coords":{"x":-0.3,"y":-0.3,"z":0}},{"name":"PRVT","desc":"Private: Stationary 3","idx":3,"mac":[4,233,229,20,63,30],"mac_str":"1e","is_connected":false,"coords":{"x":-0.6,"y":0,"z":0}}],"updated":1678710317637}';
    }

    function parseNodeDataDisplay(data: any, useSampleData = false): NodeDataDisplay {
        let jsonData = useSampleData ? JSON.parse(exampleData()) : data;
        // console.log(typeof jsonData.updated);
        console.log(jsonData.updated);
        jsonData.updated = new Date(jsonData.updated);
        // console.log("pls...")
        // console.log(new Date(jsonData.updated));
        // console.log(jsonData.updated.toLocaleTimeString());
        const nodes = jsonData as NodeDataDisplay;
        nodes.updated = new Date(nodes.updated);
        return nodes;
    }
    
    const useSampleData = false;

    async function update(): Promise<NodeDataDisplay>{
        // return 1;
        if (useSampleData){
            console.log("updating from sample data.");
            return parseNodeDataDisplay(exampleData(), useSampleData);
        } else {
            const response = await fetch('/test');
            const data = await response.json();
            return parseNodeDataDisplay(data, useSampleData);
        }
    }

    $: seconds = node_data.updated.getSeconds();

    // let rectX: number = 50;
    // let rectY: number = 50;
  

  let mounted = false;
  let updatedStr = "";
  let coordsXStr = "";
  let coordsYStr = "";

  let mouseX: number;
  let mouseY: number;
//   let map;
  onMount(() => {
    const intervalId = setInterval(async () => {
        node_data = await update();
        console.log("updated: " + node_data.updated.toLocaleTimeString());
        updatedStr = node_data.updated.toLocaleTimeString();
        counter = counter + 1;
        mounted = true;
    }, 250);

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
            <div style="height: 100%;">
                <Readouts bind:node_data bind:counter bind:mouseX bind:mouseY/>
            </div>
        </div>
        <div style="margin-left: auto; margin-right: auto;">
            <Map bind:node_data bind:counter bind:mouseX bind:mouseY/>
        </div>
    </div>
</div>
<style lang="scss">
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