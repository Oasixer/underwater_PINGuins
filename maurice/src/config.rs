use std::fs::File;
use std::io::Read;
use std::fs;
use std::path::Path;


const TIMEOUT_READ_MS: u64 = 1000;
pub const MSG_SIZE_BYTES: usize = 801;
pub const OUTGOING_CMD_SIZE_BYTES: usize = 200;
const MAX_MSG_POLL_INTERVAL_MS: u64 = 1;
const DATA_STREAM_DIR: &str = "./data_stream";
const ADC_DATA_DIR: &str = "adc_data";
const CLI_DATA_DIR: &str = "cli_data";
const PORT: u32 = 6969;

const KNOWN_TEENSY_METADATA_COUNT: usize = 4;
const TEENSY_MACS: [[u8; 6]; KNOWN_TEENSY_METADATA_COUNT] = [
    [0x00, 0x0c, 0x29, 0x00, 0x00, 0x00],
    [0x00, 0x0c, 0x29, 0x00, 0x00, 0x01],
    [0x00, 0x0c, 0x29, 0x00, 0x00, 0x02],
    [0x00, 0x0c, 0x29, 0x00, 0x00, 0x03],
];

const TEENSY_IPS: [[u8; 4]; 4] = [
    [192, 168, 1, 60],
    [192, 168, 1, 60],
    [192, 168, 1, 60],
    [192, 168, 1, 60],
];


fn grab_ip_from_file() -> String {
    let mut file = File::open("./this_host_ip").unwrap();
    let mut contents = String::new();
    file.read_to_string(&mut contents).unwrap();
    contents
}


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
        for mac in self.teensy_macs.iter() {
            if mac[5] == last_byte {
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
                for i in 0..5 {
                    mac_str.push_str(&format!("{:02x}:", mac[i]));
                }
                println!("{} [{}]", mac[5], mac_str);
            }
            return None;
        }
        return Some(full_mac);
    }
}