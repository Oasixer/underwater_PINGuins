<script lang="ts">
    import { onMount } from "svelte";

    // export let nodeImg;
    import type { Node, NodeDataDisplay } from "./node_data";
    import { pixels_per_meter, map_height_meters, map_width_meters } from "./node_data";
    // let map_div: HTMLElement;
    let last_updated: Date = new Date();

    export let node_data: NodeDataDisplay;
    export let counter: number;
    export let mouseX = 0;
    export let mouseY = 0;
    // export let i: number;
    let nodeInner: HTMLElement[]

    const height_width_icon = 32;

    function get_node_img(node: Node) {
        console.log("get_node_img");
        if (node.idx == 0) {
            return "node0.png";
        } else {
            return "node.png";
        }
    }

    function update_node(node_data: NodeDataDisplay){//(i: number) {
        console.log("updating nodes!!!!!");
        console.log(node_data);
        if (node_data.updated == last_updated) return;
        last_updated = node_data.updated;
        let nodes = node_data.nodes; 
        if (nodes == undefined) return;
        if (nodeInner == undefined) return;
        if (nodeInner.length < nodes.length) {
            console.log("return early!");
            return;
        }
        nodes[1].is_connected = false;
        for (let i = 0; i < nodes.length; i++) {
            nodeInner[i].style.left = `${nodes[i].coords.x* pixels_per_meter - height_width_icon/2}px`;
            nodeInner[i].style.top = `${nodes[i].coords.y*pixels_per_meter -  height_width_icon/2}px`;
            nodeInner[i].style.opacity = `${nodes[i].is_connected ? 1 : 0.2}`;
        }
        // nodeInner[0].style.left = `${counter*5}px`;
        // nodeDiv.style.left = `${node_data.coords.x * pixels_per_meter}px`;
        // nodeDiv.style.top = `${node_data.coords.y * pixels_per_meter}px`;
        // const { top, left } = nodeDiv.getBoundingClientRect(),
        //     centerX = left + nodeDiv.offsetWidth / 2,
        //     centerY = top + nodeDiv.offsetHeight / 2,
        //     elPosX = (node_data.coords.x - centerX),
        //     elPosY = (node_data.coords.y - centerY);
        
        //     nodeInner.style.transform = `translate3d(${elPosX}px, ${elPosY}px, 0)`;
        // requestAnimationFrame(update_node);

    }
    $: update_node(node_data);
    $: counter, node_data = node_data;
    $: console.log(node_data.updated);

    let mounted = false;

    let mapDiv: HTMLElement;
function trackMouse() {
  mapDiv.addEventListener('mousemove', setMouse);
}
let mouse = {
  x: 0,
  y: 0
};

function setMouse(event: any) {
  mouse.x = event.clientX;
  mouse.y = event.clientY;
}

function updatePosition() {
  if (isHover(mapDiv)) {
        const { top, left } = mapDiv.getBoundingClientRect();
        mouseX = mouse.x - left;
        mouseY = mouse.y - top; 
  }
}
function resetPosition() {
    mouseX = 0;
    mouseY = 0;
}
function isHover(e: any) {
  return e.parentElement.querySelector(':hover') === e;
}

  onMount(() => {
    nodeInner = [document.createElement('div'), document.createElement('div'), document.createElement('div'), document.createElement('div')];
    mounted = true;
  });

</script>


<!-- <div bind:this={map_div} class="node-div"> -->
<!-- style="margin-left: 80px; width: 500px; height: 500px; border: 2px solid black;"> -->
<div class='map-outer' style="width: {map_width_meters*pixels_per_meter}px;">
    <div bind:this={mapDiv} class='map-div' style='height: {map_height_meters*pixels_per_meter}px; width: {map_width_meters*pixels_per_meter}px'
    on:mouseenter={trackMouse}
    on:mousemove={updatePosition}
    on:mouseleave={resetPosition}
    >
        <img src="arrow3_white.png" />
        <p class="xlabel" style="">x</p>
    <!-- <div class='axis_arrow'>
    </div> -->
    {#if mounted}
        {#each node_data.nodes as node, i}
            <div bind:this={nodeInner[i]} class="node-inner">
                <!-- <img src="favicon.png" style="position: absolute; width: 32px; height: 32px; left: 100px; top: 4px;"/> -->
                <img src="{get_node_img(node)}" style="width: 32px; height: 32px;" alt=""/>
            </div>
        {/each}
    {/if}
    </div>
</div>



<style lang="scss">
    p.xlabel{
        position: relative;
        top: -32px;
        left: 465px;
        font-family: "Roboto", "Helvetica", "Arial", sans-serif;
        font-size: 24px;
        color: white;
    }
    div.map-outer{
        // position: static; 
        display: flex;
        // display: flex;
        width: fit-content;
        height: fit-content;
        // height: 700px;
        // height: 100%;
        background-color: #759ace;
        border-radius: 5px;
        padding: 30px;
        box-shadow: 0 16px 38px -12px rgb(0 0 0 / 56%), 0 4px 25px 0px rgb(0 0 0 / 12%), 0 8px 10px -5px rgb(0 0 0 / 20%);
        div.map-div{
            div.axis_arrow{
                position: absolute;
                width: 225px;
                left: 0;
                margin-right: auto;
                scale: 0.5;
                img{
                    // width: 500px;
                    // width: 100%;
                    height: 45px;
                    object-fit: cover;
                    // object-position: left 50%;
                }
            }
            border: 2px solid black;
            background-color: #96b8e6;
            position: relative;
        }
    }
    div.node-inner{
        position: absolute;
        // width: fit-content;
    }
</style>
