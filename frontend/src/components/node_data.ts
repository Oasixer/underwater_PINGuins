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
    last_ping: Date;
}
export interface NodeDataDisplay {
    nodes: Node[];
    updated: Date;
}

// export const pixels_per_meter = 50;

export const replay_map_width_meters = 22.86;
export const replay_map_height_meters = 14.655;

export const map_height_meters: number = 1.3;
export const map_width_meters: number = 1.3;

export const map_height_pixels: number = 600; // pls make this divisible by map_height_meters
// export const map_width_pixels: number = 800; // pls make this divisible by map_height_meters

export const pixels_per_meter = Math.floor(map_height_pixels / map_height_meters);

export function posToLeftRelPixels(pos: number, x: boolean) {
    return (pos*(x?1:-1) + (x ? map_width_meters : map_height_meters) / 2) * pixels_per_meter;
}
// export function posToLeftRelPMeters(pos: number, x: boolean) {
//     return (pos + (x ? map_width_meters : map_height_meters) / 2);
// }
