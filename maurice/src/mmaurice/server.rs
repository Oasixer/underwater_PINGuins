use std::time::{Duration, Instant};
use std::thread;
use std::io::{Read, ErrorKind};
use chrono::prelude::*;
use std::fs::File;
use std::io::Write;
use std::sync::mpsc::{channel, Sender, Receiver};

use crate::utils::{create_parent_directories};
use crate::config::UnknownIpAddrError;
use crate::config::MSG_SIZE_BYTES;
use super::{
    msound_player::{ SoundEffect, },
    Maurice,
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
        // maurice.sound_player.play_sound_effect(SoundEffect::StartRecording);
        let duration = self.adc_rec_metadata.duration.expect("duration not set yet???");
        self.fprint(&format!("Began recording data for {} sec\n", duration.as_secs()));
        // maurice.sound_player.play_sound_effect(SoundEffect::StartRecording);
    }
}


impl Maurice {
    pub(super) fn accept_new_connections_nonblocking(&mut self) {
        // .accept() returns Result<(TcpStream, SocketAddr)>
        if let Ok((mut socket, addr)) = self.server.accept() {
            println!("\n=============== PINGUIN CONNECTED {} ================", addr);
            Self::print_commands_info();
            socket.set_read_timeout(Some(Duration::from_millis(self.config.timeout_read_ms))).expect("Failed to set read timeout.");

            let ip_bytes = match addr.ip() {
                std::net::IpAddr::V4(ipv4) => ipv4.octets(),
                _ => panic!("Expected IPv4 address"),
            };

            let mac: [u8;6] = self.config.get_mac_from_ip(ip_bytes).expect("failed to get mac from ip");

            let cur_datetime: DateTime<Local> = Local::now();
            
            // let (tx_cmd_from_stdin_listener_to_client, rx_client_publisher) = channel::<Command>();
            let tx_msg_from_client_listener_to_consumer = self.tx_msg_from_client_listener_to_consumer.clone();
            let tx_adc_file_writer = self.tx_adc_file_writer.clone();
            let tx_connection_change = self.tx_connection_change.clone();


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
                // tx_cmd_from_stdin_listener_to_client,
            };
            // not sure if needed yet
            tx_connection_change.send(ConnectionChange{
                mac,
                is_connected: true,
                // adc_rec_metadata_changed: None
            }).expect("failed to send connection change out of channel");
            
            self.client_sockets.push(client_socket_wrapper);

            // let MSG_SIZE_BYTES = self.config.MSG_SIZE_BYTES;
            // let MAX_MSG_POLL_INTERVAL_MS = self.config.MAX_MSG_POLL_INTERVAL_MS;
            // let config = self.config.clone();

            thread::spawn(move || loop {
                // let tx_msg_from_client_listener_to_consumer
                // let mut buff = vec![0; config.msg_size_bytes];
                let mut buff: [u8; MSG_SIZE_BYTES] = [0; MSG_SIZE_BYTES];
                // let mut buff = vec![0; MSG_SIZE_BYTES];

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
        if let Ok(connection_change) = self.rx_connection_change.try_recv() {
            // println!("got a connection change to connected={} from the consumer thread: {}", connection_change.is_connected, connection_change.mac[5]);
            if connection_change.is_connected {
                self.sound_player.play_sound_effect(SoundEffect::Connect);
                match self.get_client_socket(connection_change.mac){
                    Ok(client_socket_wrapper) => {
                        client_socket_wrapper.fprint_create_file("Connected!\n");
                    }
                    Err(_) => {
                        panic!("client socket wrapper not found");
                    }
                }
            }
            else {
                self.sound_player.play_sound_effect(SoundEffect::Disconnect);
                match self.get_client_socket(connection_change.mac){
                    Ok(client_socket_wrapper) => {
                        client_socket_wrapper.fprint_create_file("Disconnected!\n");
                    }
                    Err(_) => {
                        panic!("client socket wrapper not found");
                    }
                }
            }
        }
    }
}
