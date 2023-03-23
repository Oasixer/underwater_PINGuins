use std::fs::File;
use std::io::Read;
use std::fs;
use std::path::Path;
use serde::Serialize;


const TIMEOUT_READ_MS: u64 = 1000;
pub const HB_TIMEOUT_MS: u64 = 5000;
pub const MSG_SIZE_BYTES: usize = 800;
pub const OUTGOING_CMD_SIZE_BYTES: usize = 200;
const MAX_MSG_POLL_INTERVAL_MS: u64 = 1;
pub const DATA_STREAM_DIR: &str = "../data_stream";
pub const ADC_DATA_DIR: &str = "adc_data";
pub const CLI_DATA_DIR: &str = "cli_data";
const PORT: u32 = 6969;
pub const HIDE_HB : bool = true;

const KNOWN_TEENSY_METADATA_COUNT: usize = 4;

#[derive(Debug, PartialEq, Serialize, Clone)]
pub struct Coord3D {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}
#[derive(Serialize, Clone)]
pub struct Node {
    name: String,
    desc: String,
    idx: u8,
    pub mac: [u8; 6],
    mac_str: String,
    pub is_connected: bool,
    pub coords: Coord3D,
    pub last_ping: u128,
}

const TEENSY_MACS: [[u8; 6]; KNOWN_TEENSY_METADATA_COUNT] = [
    [0x04, 0xe9, 0xe5, 0x14, 0x0f, 0x14],
    [0x04, 0xe9, 0xe5, 0x14, 0x0e, 0xf1],
    [0x04, 0xe9, 0xe5, 0x14, 0x3f, 0x41],
    [0x04, 0xe9, 0xe5, 0x14, 0x3f, 0x1e],
];

const TEENSY_IPS: [[u8; KNOWN_TEENSY_METADATA_COUNT]; KNOWN_TEENSY_METADATA_COUNT] = [
    [192, 168, 1, 100], // 14, skip
    [192, 168, 1, 103], // f1, rico
    [192, 168, 1, 102], // 41, kwsk
    [192, 168, 1, 101], // 1e, prvt
];
//14 C0,0,0;0.46,-0.46,0;0,0.65,0;-0.65,0,0
pub fn config_const_nodes() -> [Node; 4]{
    return [
        Node{
            name: "SKPR".to_string(),
            desc: "Skipper: ROV".to_string(),
            idx: 0,
            mac: TEENSY_MACS[0],
            mac_str: format!("{:02x}", TEENSY_MACS[0][5]),
            is_connected: false,
            coords: Coord3D{
                x: 0.0,
                y: 0.0,
                z: 0.0,
            },
            last_ping: 0,
        },
        Node{
            name: String::from("RICO"),
            desc: String::from("Rico: Stationary 1"),
            idx: 1,
            mac: TEENSY_MACS[1],
            mac_str: format!("{:02x}", TEENSY_MACS[1][5]),
            is_connected: false,
            coords: Coord3D{
                // x: 1.0,
                x: 0.46,
                y: -0.46,
                // y: 1.0,
                z: 0.0,
            },
            last_ping: 0,
        },
        Node{
            name: String::from("KWSK"),
            desc: String::from("Kowalski: Stationary 2"),
            idx: 2,
            mac: TEENSY_MACS[2],
            mac_str: format!("{:02x}", TEENSY_MACS[2][5]),
            is_connected: false,
            coords: Coord3D{
                x: 0.0,
                y: 0.65,
                z: 0.0,
            },
            last_ping: 0,
        },
        Node{
            name: String::from("PRVT"),
            desc: String::from("Private: Stationary 3"),
            idx: 3,
            mac: TEENSY_MACS[3],
            mac_str: format!("{:02x}", TEENSY_MACS[3][5]),
            is_connected: false,
            coords: Coord3D{
                x: -0.65,
                y: 0.0,
                z: 0.0,
            },
            last_ping: 0,
        },
    ];
}




fn grab_ip_from_file() -> String {
    let mut file = File::open("./this_host_ip").unwrap();
    let mut contents = String::new();
    file.read_to_string(&mut contents).unwrap();
    contents
}


#[derive(Clone)]
pub struct Config{
    pub timeout_read_ms: u64,
    pub msg_size_bytes: usize,
    pub outgoing_cmd_size_bytes: usize,
    pub my_ip: String,
    pub server_port: u32,
    pub max_msg_poll_interval_ms: u64,
    pub data_stream_dir: &'static str,
    pub adc_data_dir: &'static str,
    pub cli_data_dir: &'static str,
    pub teensy_macs: [[u8; 6]; KNOWN_TEENSY_METADATA_COUNT],
    pub teensy_ips: [[u8; 4]; KNOWN_TEENSY_METADATA_COUNT],
    pub known_teensy_metadata_count: usize,
}

#[derive(Debug)]
pub struct UnknownIpAddrError {
    ip: [u8; 4],
}

impl std::fmt::Display for UnknownIpAddrError {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "The specified IP address {:?} could not be found", self.ip)
    }
}

impl std::error::Error for UnknownIpAddrError {}

impl Config {
    pub fn get_mac_from_ip(&self, ip: [u8; 4]) -> Result<[u8; 6], UnknownIpAddrError> {
        for i in 0..self.teensy_ips.len() {
            if self.teensy_ips[i] == ip {
                return Ok(self.teensy_macs[i]);
            }
        }
        return Err(UnknownIpAddrError{ip});
    }
    pub fn get_desig_from_ip(&self, ip: [u8; 4]) -> Result<String, UnknownIpAddrError> {
        let nodes = config_const_nodes();
        for i in 0..self.teensy_ips.len() {
            if self.teensy_ips[i] == ip {
                return Ok(nodes[i].name.clone());
            }
        }
        return Err(UnknownIpAddrError{ip});
    }
    pub fn new() -> Self {
        let my_ip = grab_ip_from_file();
        // for teensy_mac in TEENSY_MACS.iter()
        Config{
            timeout_read_ms: TIMEOUT_READ_MS,
            msg_size_bytes: MSG_SIZE_BYTES,
            outgoing_cmd_size_bytes: OUTGOING_CMD_SIZE_BYTES,
            my_ip,
            server_port: PORT,
            max_msg_poll_interval_ms: MAX_MSG_POLL_INTERVAL_MS,
            data_stream_dir: DATA_STREAM_DIR,
            adc_data_dir: ADC_DATA_DIR,
            cli_data_dir: CLI_DATA_DIR,
            teensy_macs: TEENSY_MACS,
            teensy_ips: TEENSY_IPS,
            known_teensy_metadata_count: KNOWN_TEENSY_METADATA_COUNT,
        }
    }

    pub fn full_mac(&self, last_byte: u8) -> [u8; 6]{
        // return the mac matching the last 2 digits provided
        // println!("Looking for mac with last byte: {:02x}", last_byte);
        for mac in self.teensy_macs.iter() {
            if mac[5] == last_byte {
                // println!("Found mac: {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                return *mac;
            }
        }
        [0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
    }
    
    pub fn full_mac_or_print_available(&self, last_byte: u8) -> Option<[u8; 6]>{
        // return the mac matching the last 2 digits provided
        let full_mac = self.full_mac(last_byte);
        if full_mac == [0,0,0,0,0,0]{
            println!("Invalid target mac byte: {}", last_byte);
            println!("... valid mac bytes are: ");
            // print valid mac bytes
            for mac in self.teensy_macs.iter() {
                // serialize mac to xx:xx:xx:xx:xx:xx
                let mut mac_str = String::new();
                for i in 0..6 {
                    mac_str.push_str(&format!("{:02x}:", mac[i]));
                }
                println!("{} [{}]", mac[5], mac_str);
            }
            return None;
        }
        return Some(full_mac);
    }
}