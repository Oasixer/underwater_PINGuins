<script lang="ts">
    // let node_divs: HTMLElement[] = [node0div, node1div, node2div, node3div];
    import type { Node, NodeDataDisplay } from "./node_data";
    import { pixels_per_meter } from "./node_data";
    export let node_data: NodeDataDisplay;
    export let counter: number = 0;
    export let mouseX: number;
    export let mouseY: number;
    const READOUT_N_DIGITS = 3;
    const CONNECTED_STR = "Connected";
    const DISCONNECTED_STR = "Disconnected";

    interface SingleReadout{
        name: string;
        value: string;
    }
    
    interface Readouts{
        readouts: SingleReadout[];
    }

    let readouts: Readouts = {readouts: []};
    let conn_readouts: Readouts = {readouts: []};

    function node_data_to_readouts(node_data: NodeDataDisplay): Readouts{
        let readouts: Readouts = {readouts: []};
        if (node_data){
            if (node_data.nodes){
                if (node_data.nodes.length > 0){
                    console.log("hi!");
                    readouts.readouts.push({name: "rov x", value: node_data.nodes[0].coords.x.toFixed(READOUT_N_DIGITS)});
                    readouts.readouts.push({name: "rov y", value: node_data.nodes[0].coords.y.toFixed(READOUT_N_DIGITS)});
                }
                readouts.readouts.push({name: "updated", value: node_data.updated.toLocaleTimeString()});
            }

            let mouseX_str: string, mouseY_str: string;
            if (!mouseX){
                mouseX_str = "N/A";
                mouseY_str = "N/A";
            }
            else{
                mouseX_str = (mouseX/ pixels_per_meter).toFixed(READOUT_N_DIGITS);
                mouseY_str = (mouseY / pixels_per_meter).toFixed(READOUT_N_DIGITS);
            }
            readouts.readouts.push({name: "mouse x", value: mouseX_str});
            readouts.readouts.push({name: "mouse y", value: mouseY_str});
        }
        return readouts;
    }
    
    function node_data_to_conn_readouts(node_data: NodeDataDisplay): Readouts{
        let readouts: Readouts = {readouts: []};
        if (node_data){
            if (node_data.nodes){
                if (node_data.nodes.length > 0){
                    node_data.nodes.forEach(i => {
                       readouts.readouts.push({name: i.name, value: i.is_connected?CONNECTED_STR:DISCONNECTED_STR});
                    });
                }
            }
        }
        return readouts;
    }

    function update(counter: number){
        counter = counter;
        readouts = node_data_to_readouts(node_data);
        conn_readouts = node_data_to_conn_readouts(node_data);
    }
    $: update(counter);
</script>

<div class='vstack'>
    <div class='readouts' style="margin-bottom: 35px;">
        <h1 style="margin-bottom: 30px;">Readouts</h1>
        <div class="readout">
            <h3 class="name-width" style="font-weight: 700;">Name</h3>
            <h3 class="val-width" style="font-weight: 700;">Value</h3>
        </div>
        {#each readouts.readouts as readout}
        <div class="readout">
            <h3 class="name-width">{readout.name}</h3>
            <p class="val-width">{readout.value}</p>
        </div>
        {/each}
    </div>
    <div class='readouts' style="margin-top: auto;">
        <h1 style="margin-bottom: 30px;">Connections</h1>
        <div class="readout">
            <h3 class="name-width" style="font-weight: 700;">Node</h3>
            <h3 class="val-width" style="font-weight: 700;">Connection</h3>
        </div>
        {#each conn_readouts.readouts as readout}
        <div class="readout">
            <h3 class="name-width">{readout.name}</h3>
            <p class="val-width" style="background-color: {readout.value===CONNECTED_STR?'#163':'#a12'};">{readout.value}</p>
        </div>
        {/each}
    </div>
</div>
<style lang="scss">
    *{
        // font-family: BlinkMacSystemFont, -apple-system, "Segoe UI", "Roboto", "Oxygen", "Ubuntu", "Cantarell", "Fira Sans", "Droid Sans", "Helvetica Neue", "Helvetica", "Arial", sans-serif;
        font-family: "Roboto", "Helvetica", "Arial", sans-serif;
        font-size: 16px;
    }
    p.connected{
        background-color: #164;
    }
    p.disconnected{
        background-color: #612;
    }

    div.vstack{
        display: flex;
        flex-flow: column nowrap;
    }
    div.readouts{
        display: flex;
        flex-flow: column nowrap;
        justify-content: flex-start;
        align-items: flex-start;
        width: 260px;
        background-color: white;
        border-radius: 5px;
        padding-right: 18px;
        padding-bottom: 18px;
        margin-left: 80px;
        height: fit-content;
        box-shadow: 0 16px 38px -12px rgb(0 0 0 / 56%), 0 4px 25px 0px rgb(0 0 0 / 12%), 0 8px 10px -5px rgb(0 0 0 / 20%);
        h1 {
            text-align: center;
            margin-left: auto;
            margin-right: auto;
            height: 2em;
            font-weight: 300;
            padding: 0.5em;
            font-size: 19px;
        }
    }
    div.readout{
        display: flex;
        flex-flow: row nowrap;
        justify-content: flex-start;
        font-size: 16px;
        height: 1.5em;
        margin-bottom: 5px;
        h3{
            // color: #f7f5f4;
            // background-color: #5f5dab;
            // color: #5f5dab;
            color: #444444;
            font-weight: 300;
        }
        .name-width{
            width: 6em;
            text-align: right;
            padding-right: 0.5em;
            padding-top: 2px;
        }
        .val-width{
            width: 8em;
            text-align: left;
            padding-left: 0.5em;
            padding-top: 2px;
        }
        p{
            color: black;
            // background-color: #759ace;
            background-color: #86a8d6;
            border-radius: 4px;
        }
    }
</style>