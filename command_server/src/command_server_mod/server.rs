use std::time::{Duration, Instant};
use std::thread;
use std::io::{Read, ErrorKind};
use chrono::prelude::*;
use std::fs::File;
use std::io::Write;
use std::sync::mpsc::{channel, Sender, Receiver};

use crate::utils::{create_parent_directories};
use crate::config::{MSG_SIZE_BYTES, HB_TIMEOUT_MS, Node};
use super::{
    sound_player_mod::{ SoundEffect, },
    CommandServer,
    ConnectionChange,
    ClientSocketWrapper,
    Msg,
    AdcMsgToWrite,
    AdcRecMetadata,
    Command,
};

impl ClientSocketWrapper {
    pub fn setup_stream(&mut self){
        let file_path = &self.stream_file_name;
        create_parent_directories(file_path).expect("failed to create directories");


        let mut file = File::open(file_path);
        if let Err(e) = File::open(file_path) {
            if e.kind() == std::io::ErrorKind::NotFound {
                file = File::create(file_path);
            } else {
                panic!("unable to create stream file: {}", file_path);
            }
        }
        self.stream_file = Some(file.unwrap());
    }
    pub fn fprint(&self, msg: &str) {
        if (self.stream_file.is_none()) {
            panic!("stream_file is None");
        }
        if let Err(e) = self.stream_file.as_ref().unwrap().write_all(msg.as_bytes()) {
            eprintln!("Error writing to stream file: {}", e);
        }
        // self.stream_file
    }
    pub fn fprint_create_file(&mut self, msg: &str) {
        if (self.stream_file.is_none()) {
            self.setup_stream();
            // panic!("stream_file is None");
        }
        if let Err(e) = self.stream_file.as_ref().unwrap().write_all(msg.as_bytes()) {
            eprintln!("Error writing to stream file: {}", e);
        }
        // self.stream_file
    }

    pub fn report_begin_recording(&self) {
        // command_server.sound_player.play_sound_effect(SoundEffect::StartRecording);
        let duration = self.adc_rec_metadata.duration.expect("duration not set yet???");
        self.fprint(&format!("Began recording data for {} sec\n", duration.as_secs()));
        // command_server.sound_player.play_sound_effect(SoundEffect::StartRecording);
    }
    pub fn poll_for_disconnect(&self) -> bool{
        if self.latest_connection_change.as_ref().unwrap().is_connected == false{
            return false;
        }
        if Instant::now() - (self.last_hb_received) > Duration::from_millis(HB_TIMEOUT_MS) {
            println!("Client {:02x} timed out, last hb was {}ms ago", self.mac[5], (Instant::now() - (self.last_hb_received)).as_millis());
            // self.fprint("Disconnected.\n")
            return true;
            // self.tx_cmd_from_stdin_listener_to_client.send(Command::Disconnect).unwrap();
        }
        return false;
    }
}


impl CommandServer {
    pub(super) fn accept_new_connections_nonblocking(&mut self) {
        // .accept() returns Result<(TcpStream, SocketAddr)>
        if let Ok((mut socket, addr)) = self.server.accept() {
            let ip_bytes = match addr.ip() {
                std::net::IpAddr::V4(ipv4) => ipv4.octets(),
                _ => panic!("Expected IPv4 address"),
            };

            let mac: [u8;6] = self.config.get_mac_from_ip(ip_bytes).expect("failed to get mac from ip");
            let name: String = self.config.get_desig_from_ip(ip_bytes).expect("failed to get mac from ip");

            // get mac last byte as string using from_utf8:
            // let last_mac_byte_vec = vec![mac[5]];
            // let mac_last_byte = String::from_utf8(last_mac_byte_vec).expect("failed to convert mac last byte to string");

            if self.get_client_socket(mac).is_ok() {
                println!("\n=============== PINGUIN {} @{:02x} RE-CONNECTED ON {} ================", name, mac[5], addr);
            }
            else{
                println!("\n=============== PINGUIN {} @{:02x} CONNECTED ON {} ================", name, mac[5], addr);
            }
            Self::print_commands_info();
            socket.set_read_timeout(Some(Duration::from_millis(self.config.timeout_read_ms))).expect("Failed to set read timeout.");


            let cur_datetime: DateTime<Local> = Local::now();
            
            // let (tx_cmd_from_stdin_listener_to_client, rx_client_publisher) = channel::<Command>();
            let tx_msg_from_client_listener_to_consumer = self.tx_msg_from_client_listener_to_consumer.clone();
            let tx_adc_file_writer = self.tx_adc_file_writer.clone();
            let tx_connection_change = self.tx_connection_change.clone();


            let (kill_me_tx, kill_me_rx) = channel::<bool>();
            let client_socket_wrapper: ClientSocketWrapper = ClientSocketWrapper {
                mac,
                latest_connection_change: Some(ConnectionChange{
                    mac,
                    is_connected: true,
                }),
                    // adc_rec_metadata_changed: None}),
                stream: socket.try_clone().expect("failed to clone client: wtf... how?"),
                stream_file_name: format!("{}/{}/{}/{}", self.config.data_stream_dir, mac.iter().map(|b| format!("{:02x}", b)).collect::<String>(), self.config.cli_data_dir, cur_datetime.format("%Y-%m-%d_%H-%M-%S")),
                stream_file: None,
                adc_rec_metadata: AdcRecMetadata{
                    socket_mac: mac,
                    tx_adc_file_writer,
                    file_prefix: None,
                    file_suffix: None,
                    file_name: None,
                    duration: None,
                    end_time: None,
                },
                last_hb_received: Instant::now(),
                kill_me: kill_me_tx,
                // tx_cmd_from_stdin_listener_to_client,
            };
            // not sure if needed yet
            tx_connection_change.send(ConnectionChange{
                mac,
                is_connected: true,
                // adc_rec_metadata_changed: None
            }).expect("failed to send connection change out of channel");

            
            
            let prev_socket = self.get_client_socket(mac);
            if prev_socket.is_ok() { //
                self.replace_client_socket(mac, client_socket_wrapper);
            }
            else{
                self.client_sockets.push(client_socket_wrapper);
            }

            // let MSG_SIZE_BYTES = self.config.MSG_SIZE_BYTES;
            // let MAX_MSG_POLL_INTERVAL_MS = self.config.MAX_MSG_POLL_INTERVAL_MS;
            // let config = self.config.clone();

            thread::spawn(move || loop {
                // let tx_msg_from_client_listener_to_consumer
                // let mut buff = vec![0; config.msg_size_bytes];
                let mut buff: [u8; MSG_SIZE_BYTES] = [0; MSG_SIZE_BYTES];
                // let mut buff = vec![0; MSG_SIZE_BYTES];
                if let Ok(_) = kill_me_rx.try_recv() {
                    println!("killing client listener thread for client: {:02x}", mac[5]);
                    break;
                }

                match socket.read_exact(&mut buff) {
                    Ok(_) => {
                        tx_msg_from_client_listener_to_consumer.send(Msg{
                            mac,
                            bytes: buff,
                        }).expect("failed to send message out of channel");
                    }
                    // socket should be blocking, so this should never happen?
                    Err(ref err) if err.kind() == ErrorKind::WouldBlock => (),
                    Err(err) => {
                        println!("Closing connection with: {} due to {}", addr, err);
                        tx_connection_change.send(ConnectionChange{
                            mac,
                            is_connected: false,
                            // adc_rec_metadata_changed: None
                        }).expect("failed to send connection change out of channel");
                        break;
                    }
                }
                // socket.flush().expect("failed to flush socket");
                // sleep causes fuckery here
                // sleep(config.max_msg_poll_interval_ms);
                // sleep(MAX_MSG_POLL_INTERVAL_MS);
            });
        }
    }

    pub(super) fn poll_for_connection_changes(&mut self){
        for client in self.client_sockets.iter_mut() {
            if client.poll_for_disconnect(){
                client.latest_connection_change = Some(ConnectionChange{
                    mac: client.mac,
                    is_connected: false,
                    // adc_rec_metadata_changed: None
                });
                client.fprint_create_file("Client timed out!\n");
                client.kill_me.send(true).expect("failed to send kill me signal to client listener thread");
                self.sound_player.play_sound_effect(SoundEffect::Disconnect);
                // match self.get_client_socket(connection_change.mac){
                //     Ok(client_socket_wrapper) => {
                //         client_socket_wrapper.fprint_create_file("Disconnected!\n");
                //     }
                //     Err(_) => {
                //         panic!("client socket wrapper not found");
                //     }
                // }
            }
        }
        if let Ok(connection_change) = self.rx_connection_change.try_recv() {
            // println!("got a connection change to connected={} from the consumer thread: {}", connection_change.is_connected, connection_change.mac[5]);
            let mac: [u8; 6] = connection_change.mac.clone();
            match self.get_client_socket(connection_change.mac){
                Ok(client_socket_wrapper) => {
                    let sound_effect: SoundEffect;
                    if connection_change.is_connected {
                        sound_effect = SoundEffect::Connect;
                        // self.sound_player.play_sound_effect(SoundEffect::Connect);
                        client_socket_wrapper.fprint_create_file("Connected!\n");

                        // if client_socket_wrapper.mac[5] == (0x14 as u8){
                            
                            // let data = self.data_provided_to_frontend.lock().unwrap();
                            // let coords_string = generate_coords_string(&data.nodes);
                            // let cmd_str = format!("14 C{}", coords_string);
                            // // client_socket_wrapper.
                            // // let cmd_str: String = tokens.join(" ");
                            // // fill cmd_bytes in with cmd_str, right padded w/ zeros
                            // // const msg_size_bytes: usize = self.config.OUTGOING_CMD_SIZE_BYTES;
                            // let mut cmd_bytes: [u8; super::OUTGOING_CMD_SIZE_BYTES] = [0; super::OUTGOING_CMD_SIZE_BYTES];
                            // for (i,byte) in cmd_str.bytes().enumerate(){
                            //     cmd_bytes[i] = byte;
                            // }

                            // // if not a w command, then its a string command that we will allow the teensy to handle.
                            // let mac_str = format!("{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                            // let command = Command{
                            //     target_mac: mac,
                            //     bytes: cmd_bytes,
                            // };
                            // // let command = Command{
                            // //     target_mac: client_socket_wrapper.mac,
                            // //     bytes: 
                            // // }
                            // // self.report_cmd(&format!("Sending command: <{}> to {}",cmd_str, mac_str));
                            // let result = client_socket_wrapper.stream.write_all(&command.bytes);
                        // }
                    }
                    else{
                        sound_effect = SoundEffect::Disconnect;
                        // self.sound_player.play_sound_effect(SoundEffect::Disconnect);
                        client_socket_wrapper.fprint_create_file("Disconnected!\n");
                    }
                    self.sound_player.play_sound_effect(sound_effect);
                }
                Err(_) => {
                    panic!("client socket wrapper not found");
                }
            }
            let mut data = self.data_provided_to_frontend.lock().unwrap();
            let node = data.nodes.iter_mut().find(|node| node.mac == mac).ok_or_else(|| format!("Could not find client socket with mac: {:?}", mac));
            node.unwrap().is_connected = connection_change.is_connected;
        }
    }
}

fn generate_coords_string(nodes: &[Node]) -> String {
    let mut coords_string = String::new();
    for node in nodes {
        let coords = &node.coords;
        coords_string.push_str(&format!("{};{};{}", coords.x, coords.y, coords.z));
        coords_string.push(';');
    }
    coords_string.pop(); // Remove the trailing ';'
    coords_string
}
