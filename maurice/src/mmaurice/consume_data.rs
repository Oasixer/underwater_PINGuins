use std::net::TcpStream;
use std::io::{Read, Write};
use std::io::ErrorKind;
use std::time::Instant;

use super::{
    Maurice,
    ClientSocketWrapper,
    AdcMsgToWrite,
    // DisplayData,
    Coord3D,
    MSG_SIZE_BYTES,
    msound_player::{ SoundEffect, },
};
use crate::utils::create_parent_directories;


impl Maurice {
    pub(super) fn poll_for_finished_recording(&mut self){
        // iterate self.client_sockets
        for client_socket_wrapper in self.client_sockets.iter_mut() {
            if client_socket_wrapper.adc_rec_metadata.end_time.is_some() {
                if Instant::now() > client_socket_wrapper.adc_rec_metadata.end_time.unwrap() {
                    if (client_socket_wrapper.adc_rec_metadata.file_prefix.is_some()){
                        client_socket_wrapper.fprint(&format!("Finished recording to file: {}_{} for {} sec\n",
                            client_socket_wrapper.adc_rec_metadata.file_prefix.as_ref().unwrap(), client_socket_wrapper.adc_rec_metadata.file_suffix.unwrap(), client_socket_wrapper.adc_rec_metadata.duration.unwrap().as_secs())); 
                            client_socket_wrapper.adc_rec_metadata.file_suffix = Some(client_socket_wrapper.adc_rec_metadata.file_suffix.unwrap() + 1);
                    }
                    else{
                    client_socket_wrapper.fprint(&format!("Finished recording to file: {} for {} sec\n",
                        client_socket_wrapper.adc_rec_metadata.file_name.as_ref().unwrap(), client_socket_wrapper.adc_rec_metadata.duration.unwrap().as_secs())); 
                    }

                    client_socket_wrapper.adc_rec_metadata.file_name = None;
                    client_socket_wrapper.adc_rec_metadata.end_time = None;

                    self.sound_player.play_sound_effect(SoundEffect::EndRecording);
                }
            }
        }
    }

    pub(super) fn poll_for_messages_passed_from_socket_polling_threads(&mut self) {
        if let Ok(msg) = self.rx_msg_consumer.try_recv() {
            let config = self.config.clone();
            let mac: [u8;6] = msg.mac;
            // println!("Got a message from {}", mac[5]);
            // print first 10 bytes of message
            // for i in 0..800 {
            //     print!("{:02x} ", msg.bytes[i]);
            // }
            // println!("");
            // let client_socket_wrapper: &mut ClientSocketWrapper = self.get_client_socket(mac).unwrap();
            let client_socket_wrapper: Result<&mut ClientSocketWrapper, String> = self.get_client_socket(mac);
            match client_socket_wrapper {
                Err(e) => {
                    panic!("Somehow we lied about what MAC we received shit from {}", e);    
                }
                Ok(client_socket_wrapper) => {
                    // let client_socket_wrapper = client_socket_wrapper;
                    if client_socket_wrapper.stream_file.is_none() {
                        client_socket_wrapper.fprint_create_file("Hi.");
                    }
                    if msg.bytes[0] & 0b11110000 == 0b00000000 { // flag for adc message
                        if client_socket_wrapper.adc_rec_metadata.end_time.is_some(){
                            let adc_rec_metadata = client_socket_wrapper.adc_rec_metadata.clone();
                            let mac_str = mac.iter().map(|b| format!("{:02x}", b)).collect::<String>(); 
                            let mut file_path_to_write= String::new();
                            if adc_rec_metadata.file_prefix.is_some() && adc_rec_metadata.end_time.is_some() {
                                let file_prefix = adc_rec_metadata.file_prefix.unwrap();
                                let file_suffix = adc_rec_metadata.file_suffix.unwrap();
                                file_path_to_write = format!("{}/{}/{}/{}_{}", config.data_stream_dir, mac_str, config.adc_data_dir, file_prefix, file_suffix);
                            }
                            else if adc_rec_metadata.file_name.is_some() {
                                let file_name = adc_rec_metadata.file_name.unwrap();
                                file_path_to_write = format!("{}/{}/{}/{}", config.data_stream_dir, mac_str, config.adc_data_dir, file_name);
                            }
                            create_parent_directories(&file_path_to_write).expect("failed to create directories");
                            let msg_to_write: AdcMsgToWrite = AdcMsgToWrite{
                                file_path: file_path_to_write,
                                msg,
                            };
                            client_socket_wrapper.adc_rec_metadata.tx_adc_file_writer.send(msg_to_write).expect("failed to send message to adc file writer");
                        }
                    }
                    
                    // check if first 4 bits are 0b1001
                    // else if msg.bytes[0] & 0b11110000 == 0b01100000 { // flag for string message
                    else if msg.bytes[0] == 0b10010000 { // flag for string message
                        // append remaining 799? bytes to stream_file
                        // find last non-zero byte in msg
                        // println!("got a string message from {}", mac[5]);
                        let last_non_zero_byte = msg.bytes.iter().rposition(|&b| b != 0).unwrap();
                        // writeln!(client_socket_wrapper.stream_file.as_mut().unwrap(), "{}", String::from_utf8(msg.bytes[1..last_non_zero_byte+1].to_vec()).unwrap()).expect("failed to write to file");
                        let msg_str = String::from_utf8(msg.bytes[1..last_non_zero_byte+1].to_vec()).unwrap();
                        client_socket_wrapper.fprint(&format!("<{}", &msg_str));

                        if msg_str.starts_with("Distances:"){
                            match parse_estimate(&msg_str) {
                                Ok(position) => {
                                    // Do something with the parsed position...
                                    // self.sound_player.play_sound_effect(SoundEffect::
                                    println!("Parsed position: ({}, {}, {})", position.x, position.y, position.z);
                                },
                                Err(err) => {
                                    eprintln!("Error parsing estimate: {}", err);
                                },
                            }
                        }
                        else if msg_str.starts_with("HB["){
                            client_socket_wrapper.last_hb_received = Instant::now();
                            // if msg_str.starts_with("HB: ROV"){

                            // }
                        }
                    }
                    else if msg.bytes[0] == 0b11111111 { // flag for leak message
                        // find last non-zero byte in msg
                        client_socket_wrapper.fprint("LEAK DETECTED!!!!\n");
                        self.sound_player.play_sound_effect(SoundEffect::Leak);
                        // self.sound_player.play_sound_effect(SoundEffect::Leak);
                    }
                } // client_socket_wrapper found
            } // match client_socket_wrapper
        }
        // sleep(self.config.max_msg_poll_interval_ms);
    }
} 

fn parse_estimate(info: &str) -> Result<Coord3D, String> {
    if let Some(start_idx) = info.find("Estimate: [") {
        let end_idx = info[start_idx..].find(']').map(|idx| start_idx + idx)
            .ok_or_else(|| "Failed to find end of Estimate field".to_string())?;
        let coords = info[start_idx + 12..end_idx].split(", ").collect::<Vec<&str>>();
        if coords.len() == 3 {
            let x = coords[0].parse().map_err(|e| format!("Failed to parse x value: {}", e))?;
            let y = coords[1].parse().map_err(|e| format!("Failed to parse y value: {}", e))?;
            let z = coords[2].parse().map_err(|e| format!("Failed to parse z value: {}", e))?;
            Ok(Coord3D { x, y, z })
        } else {
            Err("Invalid number of coordinates in Estimate field".to_string())
        }
    } else {
        Err("Failed to find Estimate field".to_string())
    }
}