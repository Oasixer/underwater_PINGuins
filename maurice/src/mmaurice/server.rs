use std::time::{Duration, Instant};
use std::thread;
use std::io::{Read, ErrorKind};
use chrono::prelude::*;
use std::fs::File;
use std::io::Write;

use crate::utils::create_directories;
use crate::config::UnknownIpAddrError;
use crate::config::MSG_SIZE_BYTES;
use super::{
    msound_player::{ SoundEffect, },
    Maurice,
    ConnectionChange,
    ClientSocketWrapper,
    Msg,
    MsgToWrite,
    AdcRecMetadata,
};

impl Maurice {
    pub(super) fn accept_new_connections_nonblocking(&mut self) {
        // .accept() returns Result<(TcpStream, SocketAddr)>
        if let Ok((mut socket, addr)) = self.server.accept() {
            println!("\n=============== PINGUIN CONNECTED {} ================", addr);
            Self::print_commands_info();
            self.sound_player.play_sound_effect(SoundEffect::Connect);
            // conn.as_ref().unwrap().set_read_timeout(Some(Duration::from_millis(TIMEOUT_READ_MS))).expect("Failed to set read timeout.");

            socket.set_read_timeout(Some(Duration::from_millis(self.config.timeout_read_ms))).expect("Failed to set read timeout.");

            let tx_msg_from_client_listener_to_consumer = self.tx_msg_from_client_listener_to_consumer.clone();
            let tx_file_writer = self.tx_file_writer.clone();
            let tx_connection_change = self.tx_connection_change.clone();

            let ip_bytes = match addr.ip() {
                std::net::IpAddr::V4(ipv4) => ipv4.octets(),
                _ => panic!("Expected IPv4 address"),
            };

            let mac: [u8;6] = self.config.get_mac_from_ip(ip_bytes).expect("failed to get mac from ip");

            let cur_datetime: DateTime<Local> = Local::now();
            let client_socket_wrapper: ClientSocketWrapper = ClientSocketWrapper {
                mac,
                latest_connection_change: Some(ConnectionChange{
                    mac,
                    is_connected: true,
                }),
                    // adc_rec_metadata_changed: None}),
                stream: socket.try_clone().expect("failed to clone client: wtf... how?"),
                stream_file_name: format!("{}/{}/{}", self.config.data_stream_dir, mac.iter().map(|b| format!("{:02x}", b)).collect::<String>(), cur_datetime.format("%Y-%m-%d_%H-%M-%S")),
                stream_file: None,
                adc_rec_metadata: AdcRecMetadata{
                    socket_mac: mac,
                    tx_file_writer,
                    file_prefix: None,
                    file_suffix: None,
                    file_name: None,
                    duration: None,
                    end_time: None,
                },
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
            let config = self.config.clone();

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
                    Err(_) => {
                        println!("Closing connection with: {}", addr);
                        tx_connection_change.send(ConnectionChange{
                            mac,
                            is_connected: false,
                            // adc_rec_metadata_changed: None
                        }).expect("failed to send connection change out of channel");
                        break;
                    }
                }
                sleep(config.max_msg_poll_interval_ms);
                // sleep(MAX_MSG_POLL_INTERVAL_MS);
            });
        }
    }

    pub(super) fn poll_for_messages_passed_from_socket_polling_threads(&mut self) {
        if let Ok(msg) = self.rx_msg_consumer.try_recv() {
            let mac: [u8;6] = msg.mac;
            println!("Got a message from {}", mac[5]);
            let client_socket_wrapper: Result<&mut ClientSocketWrapper, String> = self.get_client_socket(mac);
            match client_socket_wrapper {
                Err(e) => {
                    panic!("Somehow we lied about what MAC we received shit from {}", e);    
                }
                Ok(client_socket_wrapper) => {
                    // let client_socket_wrapper = client_socket_wrapper;
                    if client_socket_wrapper.stream_file.is_none() {
                        let file_path = &client_socket_wrapper.stream_file_name;
                        create_directories(file_path).expect("failed to create directories");


                        let mut file = File::open(file_path);
                        if let Err(e) = File::open(file_path) {
                            if e.kind() == std::io::ErrorKind::NotFound {
                                file = File::create(file_path);
                            } else {
                                panic!("unable to create stream file: {}", file_path);
                            }
                        }
                        client_socket_wrapper.stream_file = Some(file.unwrap());
                    }
                    if msg.bytes[0] == 0b00000000 { // flag for adc message
                        let mac_str = mac.iter().map(|b| format!("{:02x}", b)).collect::<String>(); 
                        // let adc_rec_metadata = client_socket_wrapper.adc_rec_metadata.as_mut().unwrap();
                        if client_socket_wrapper.adc_rec_metadata.end_time.is_some() {
                            if Instant::now() > client_socket_wrapper.adc_rec_metadata.end_time.unwrap() {
                            // client_socket_wrapper.adc_rec_metadata = None;
                            // adc_rec_metadata.stream_file
                                // self.sound_player.play_sound_effect(SoundEffect::EndRecording);
                                // self.sound_player_
                                
                                client_socket_wrapper.stream2("Finished recording!");
                                client_socket_wrapper.adc_rec_metadata.file_name = None;
                                client_socket_wrapper.adc_rec_metadata.end_time = None;
                            }
                            client_socket_wrapper.adc_rec_metadata.write(msg.bytes);//, tx_file_writer);
                            //     let file_prefix = adc_rec_metadata.file_prefix.unwrap();
                            //     let file_suffix = adc_rec_metadata.file_suffix.unwrap();
                            //     let file_path = format!("{}/{}/{}/{}/{}{}", self.config.data_stream_dir, mac_str, self.config.adc_data_dir, file_prefix, file_suffix.as_str());
                            //     create_directories(file_path).expect("failed to create directories");
                            //     let mut file = File::open(file_path);
                            //     if let Err(e) = File::open(file_path) {
                            //         if e.kind() == std::io::ErrorKind::NotFound {
                            //             file = File::create(file_path);
                            //         } else {
                            //             panic!("unable to create adc file: {}", file_path);
                            //         }
                            //     }
                            //     let mut file = file.unwrap();
                            //     file.write_all(&msg.bytes[1..]).expect("failed to write to file");
                        }
                            // } else
                            // if adc_rec_metadata.file_prefix.is_some() {
                            //     let file_prefix = adc_rec_metadata.file_prefix.unwrap();
                            //     let file_suffix = adc_rec_metadata.file_suffix.unwrap();
                            //     let file_path = format!("{}/{}/{}/{}/{}{}", self.config.data_stream_dir, mac_str, self.config.adc_data_dir, file_prefix, file_suffix.as_str());
                            //     create_directories(file_path).expect("failed to create directories");
                            //     let mut file = File::open(file_path);
                            //     if let Err(e) = File::open(file_path) {
                            //         if e.kind() == std::io::ErrorKind::NotFound {
                            //             file = File::create(file_path);
                            //         } else {
                            //             panic!("unable to create adc file: {}", file_path);
                            //         }
                            //     }
                            //     let mut file = file.unwrap();
                            //     file.write_all(&msg.bytes[1..]).expect("failed to write to file");
                            // }
                            // client_socket_wrapper.adc_rec_metadata = Some(AdcRecMetadata::new());
                        // }
                        // let msg_to_write: MsgToWrite{
                            // file_path: format!("{}/{}", self.config.data_stream_dir, "adc"),
                        // }
                    }
                    
                    // check if first 4 bits are 0b0110
                    if msg.bytes[0] & 0b11110000 == 0b01100000 { // flag for string message
                        // append remaining 800 bytes to stream_file
                        // find last non-zero byte in msg
                        let last_non_zero_byte = msg.bytes.iter().rposition(|&b| b != 0).unwrap();
                        writeln!(client_socket_wrapper.stream_file.as_mut().unwrap(), "{}", String::from_utf8(msg.bytes[1..last_non_zero_byte+1].to_vec()).unwrap()).expect("failed to write to file");
                    }
                } // client_socket_wrapper found
            } // match client_socket_wrapper
        }
        sleep(self.config.max_msg_poll_interval_ms);
    }
    pub(super) fn poll_for_connection_changes(&mut self){
        if let Ok(connection_change) = self.rx_connection_change.try_recv() {
            println!("got a connection change from the consumer thread: {}", connection_change.mac[0]);
            if connection_change.is_connected {
                self.sound_player.play_sound_effect(SoundEffect::Connect);
            }
            //     .into_iter()
            //     .filter_map(|mut client| {
            //         let mut buff = msg.clone().into_bytes();
            //         buff.resize(MSG_SIZE, 0);

            //         match client.write_all(&buff).map(|_| client) {
            //             Ok(client) => Some(client),
            //             Err(_) => {
            //                 println!("Closing connection with: {}", addr);
            //                 self.sound_player.play_sound(SoundEffect::Disconnect).unwrap();
            //                 None
            //             }
            //         }
            //     })
            //     .collect::<Vec<_>>();
        }
        sleep(self.config.max_msg_poll_interval_ms);
    }
}

fn sleep(millis: u64) {
    thread::sleep(std::time::Duration::from_millis(millis));
}