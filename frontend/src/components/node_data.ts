export interface Coords {
	x: number;
	y: number;
	z: number;
	meta?: string;
}
export interface Node {
	name: string;
	desc: string;
	idx: number;
	mac: number[];
	mac_str: string;
	is_connected: boolean;
	// coords: {
	// 	x: number;
	// 	y: number;
	// 	z: number;
	// };
	coords: Coords;
	last_ping: Date;
}
export interface NodeDataDisplay {
	nodes: Node[];
	updated: Date;
}

// export const pixels_per_meter = 50;
//

export interface MapData {
	width_meters: number; // = 22.86;
	height_meters: number; // = 14.655;
	width_pixels: number;
	height_pixels: number; // = 600; // pls make this divisible by map_height_meters
	pixels_per_meter: number;
}

let roundMapWidthPixels = 600;
let roundMapWidthMeters = 1.3;
let roundMapPixelsPerMeter = roundMapWidthPixels / roundMapWidthMeters;
export const roundMap: MapData = {
	width_meters: roundMapWidthMeters,
	height_meters: roundMapWidthMeters,
	width_pixels: roundMapWidthMeters * roundMapPixelsPerMeter,
	height_pixels: roundMapWidthMeters * roundMapPixelsPerMeter,
	pixels_per_meter: roundMapPixelsPerMeter
};

let replayMapWidthPixels = 600;
let replayMapWidthMeters = 22.86;
let replayMapHeightMeters = 14.655;
let replayMapPixelsPerMeter = replayMapWidthPixels / replayMapWidthMeters;
let replayMapHeightPixels = Math.floor(replayMapHeightMeters * replayMapPixelsPerMeter);
export const replayMap: MapData = {
	width_meters: replayMapWidthMeters,
	height_meters: replayMapHeightMeters,
	width_pixels: replayMapWidthMeters * replayMapPixelsPerMeter,
	height_pixels: replayMapHeightMeters * replayMapPixelsPerMeter,
	pixels_per_meter: replayMapPixelsPerMeter
};

export const rov_positions_pac: Coords[] = [
	{ x: 3.73, y: 12.05, z: 0.0, meta: 'o1514' },
	{ x: 3.73, y: 12.05, z: 0.0, meta: 'X1515' },
	{ x: 3.65, y: 11.47, z: 0.0, meta: 'o1516' },
	{ x: 3.18, y: 11.24, z: 0.0, meta: 'o1517' },
	{ x: 3.47, y: 11.91, z: 0.0, meta: 'o1518' },
	{ x: 3.25, y: 11.4, z: 0.0, meta: 'o1519' },
	{ x: 3.39, y: 11.91, z: 0.0, meta: 'o1520' },
	{ x: 4.15, y: 12.14, z: 0.0, meta: 'o1521' },
	{ x: 4.69, y: 12.27, z: 0.0, meta: 'o1522' },
	{ x: 5.07, y: 12.32, z: 0.0, meta: 'o1523' },
	{ x: 5.43, y: 12.36, z: 0.0, meta: 'o1524' },
	{ x: 5.47, y: 12.35, z: 0.0, meta: 'o1525' },
	{ x: 4.86, y: 11.95, z: 0.0, meta: 'o1526' },
	{ x: 4.99, y: 11.93, z: 0.0, meta: 'o1527' },
	{ x: 5.95, y: 11.97, z: 0.0, meta: 'o1528' },
	{ x: 6.03, y: 12.28, z: 0.0, meta: 'o1529' },
	{ x: 6.47, y: 12.35, z: 0.0, meta: 'o1530' },
	{ x: 6.38, y: 12.18, z: 0.0, meta: 'o1531' },
	{ x: 7.05, y: 12.35, z: 0.0, meta: 'o1532' },
	{ x: 7.3, y: 12.4, z: 0.0, meta: 'o1533' },
	{ x: 7.72, y: 12.5, z: 0.0, meta: 'o1534' },
	{ x: 7.89, y: 12.4, z: 0.0, meta: 'o1535' },
	{ x: 8.45, y: 12.55, z: 0.0, meta: 'o1536' },
	{ x: 8.45, y: 12.55, z: 0.0, meta: 'X1537' },
	{ x: 8.29, y: 12.2, z: 0.0, meta: 'X1538' },
	{ x: 8.45, y: 12.15, z: 0.0, meta: 'X1539' },
	{ x: 8.63, y: 11.6, z: 0.0, meta: 'X1540' },
	{ x: 8.95, y: 10.8, z: 0.0, meta: 'X1541' },
	{ x: 8.48, y: 10.8, z: 0.0, meta: 'X1542' },
	{ x: 9.45, y: 9.62, z: 0.0, meta: 'X1543' },
	{ x: 9.92, y: 10.13, z: 0.0, meta: 'o1544' },
	{ x: 10.32, y: 11.17, z: 0.0, meta: 'o1545' },
	{ x: 11.98, y: 10.02, z: 0.0, meta: 'o1546' },
	{ x: 12.09, y: 10.48, z: 0.0, meta: 'o1547' },
	{ x: 12.09, y: 10.48, z: 0.0, meta: 'X1548' },
	{ x: 12.95, y: 11.54, z: 0.0, meta: 'o1549' },
	{ x: 12.95, y: 11.54, z: 0.0, meta: 'X1550' },
	{ x: 13.08, y: 11.87, z: 0.0, meta: 'o1551' },
	{ x: 13.08, y: 11.87, z: 0.0, meta: 'X1552' },
	{ x: 13.56, y: 11.8, z: 0.0, meta: 'o1553' },
	{ x: 13.56, y: 10.2, z: 0.0, meta: 'X1554' },
	{ x: 14.87, y: 9.85, z: 0.0, meta: 'o1555' },
	{ x: 14.74, y: 9.86, z: 0.0, meta: 'o1556' },
	{ x: 13.77, y: 10.74, z: 0.0, meta: 'o1557' },
	{ x: 13.77, y: 10.74, z: 0.0, meta: 'X1558' },
	{ x: 13.81, y: 9.88, z: 0.0, meta: 'o1559' },
	{ x: 13.33, y: 10.05, z: 0.0, meta: 'o1560' },
	{ x: 14.01, y: 9.17, z: 0.0, meta: 'o1561' },
	{ x: 13.78, y: 9.28, z: 0.0, meta: 'o1562' },
	{ x: 14.22, y: 9.25, z: 0.0, meta: 'o1563' },
	{ x: 14.52, y: 9.33, z: 0.0, meta: 'o1564' },
	{ x: 14.52, y: 9.33, z: 0.0, meta: 'X1565' },
	{ x: 14.82, y: 9.62, z: 0.0, meta: 'o1566' },
	{ x: 15.19, y: 9.47, z: 0.0, meta: 'o1567' },
	{ x: 15.28, y: 9.43, z: 0.0, meta: 'o1568' },
	{ x: 15.08, y: 9.53, z: 0.0, meta: 'o1569' },
	{ x: 15.04, y: 9.56, z: 0.0, meta: 'o1570' },
	{ x: 15.04, y: 9.56, z: 0.0, meta: 'X1571' },
	{ x: 15.5, y: 9.65, z: 0.0, meta: 'o1572' },
	{ x: 15.57, y: 9.84, z: 0.0, meta: 'o1573' },
	{ x: 15.96, y: 9.95, z: 0.0, meta: 'o1574' },
	{ x: 16.33, y: 10.1, z: 0.0, meta: 'o1575' },
	{ x: 16.94, y: 10.09, z: 0.0, meta: 'o1576' },
	{ x: 16.94, y: 10.09, z: 0.0, meta: 'X1577' },
	{ x: 17.51, y: 9.99, z: 0.0, meta: 'o1578' },
	{ x: 18.05, y: 11.21, z: 0.0, meta: 'o1579' },
	{ x: 18.34, y: 10.08, z: 0.0, meta: 'o1580' },
	{ x: 19.0, y: 9.89, z: 0.0, meta: 'o1581' },
	{ x: 19.49, y: 10.06, z: 0.0, meta: 'o1582' },
	{ x: 19.49, y: 10.06, z: 0.0, meta: 'X1583' },
	{ x: 19.91, y: 10.4, z: 0.0, meta: 'o1584' },
	{ x: 18.89, y: 9.94, z: 0.0, meta: 'o1585' },
	{ x: 19.02, y: 9.88, z: 0.0, meta: 'o1586' },
	{ x: 19.64, y: 9.53, z: 0.0, meta: 'o1587' },
	{ x: 19.64, y: 9.53, z: 0.0, meta: 'X1588' },
	{ x: 19.52, y: 9.88, z: 0.0, meta: 'o1589' },
	{ x: 19.28, y: 10.18, z: 0.0, meta: 'o1590' },
	{ x: 19.3, y: 10.55, z: 0.0, meta: 'o1591' },
	{ x: 18.59, y: 10.93, z: 0.0, meta: 'o1592' },
	{ x: 18.77, y: 10.81, z: 0.0, meta: 'o1593' },
	{ x: 18.77, y: 10.81, z: 0.0, meta: 'X1594' },
	{ x: 18.77, y: 10.81, z: 0.0, meta: 'XX1595' },
	{ x: 18.14, y: 11.47, z: 0.0, meta: 'o1596' },
	{ x: 18.14, y: 11.47, z: 0.0, meta: 'X1597' },
	{ x: 19.64, y: 11.99, z: 0.0, meta: 'o1598' },
	{ x: 18.23, y: 11.43, z: 0.0, meta: 'o1599' },
	{ x: 18.44, y: 11.59, z: 0.0, meta: 'o1600' },
	{ x: 19.22, y: 12.22, z: 0.0, meta: 'o1601' },
	{ x: 19.1, y: 12.16, z: 0.0, meta: 'o1602' },
	{ x: 18.8, y: 11.72, z: 0.0, meta: 'o1603' },
	{ x: 18.72, y: 11.85, z: 0.0, meta: 'o1604' },
	{ x: 18.48, y: 11.74, z: 0.0, meta: 'o1605' },
	{ x: 18.43, y: 12.28, z: 0.0, meta: 'o1606' },
	{ x: 18.85, y: 12.0, z: 0.0, meta: 'o1607' },
	{ x: 18.85, y: 12.0, z: 0.0, meta: 'X1608' },
	{ x: 18.73, y: 11.61, z: 0.0, meta: 'o1609' },
	{ x: 18.47, y: 11.17, z: 0.0, meta: 'o1610' },
	{ x: 18.66, y: 10.76, z: 0.0, meta: 'o1611' },
	{ x: 18.36, y: 10.76, z: 0.0, meta: 'X1612' },
	{ x: 17.21, y: 10.27, z: 0.0, meta: 'o1613' },
	{ x: 17.13, y: 9.9, z: 0.0, meta: 'o1614' },
	{ x: 16.47, y: 10.07, z: 0.0, meta: 'o1615' },
	{ x: 15.35, y: 9.98, z: 0.0, meta: 'o1616' },
	{ x: 14.38, y: 10.24, z: 0.0, meta: 'o1617' },
	{ x: 15.68, y: 10.07, z: 0.0, meta: 'o1618' },
	{ x: 13.73, y: 10.09, z: 0.0, meta: 'o1619' },
	{ x: 13.26, y: 10.03, z: 0.0, meta: 'o1620' },
	{ x: 13.01, y: 9.8, z: 0.0, meta: 'o1621' },
	{ x: 12.36, y: 9.8, z: 0.0, meta: 'X1622' },
	{ x: 12.53, y: 11.4, z: 0.0, meta: 'o1623' },
	{ x: 11.81, y: 11.06, z: 0.0, meta: 'o1624' },
	{ x: 10.61, y: 11.06, z: 0.0, meta: 'X1625' },
	{ x: 9.81, y: 11.06, z: 0.0, meta: 'XX1626' },
	{ x: 9.68, y: 11.31, z: 0.0, meta: 'o1627' }
];
