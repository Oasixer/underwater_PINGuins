export interface Node{
    name: string;
    desc: string;
    idx: number;
    mac: number[];
    mac_str: string;
    is_connected: boolean;
    coords: {
        x: number;
        y: number;
        z: number;
    };
}
export interface NodeDataDisplay {
    nodes: Node[];
    updated: Date;
}

export const pixels_per_meter = 50;

export const map_width_meters = 22.86;
export const map_height_meters = 14.655;