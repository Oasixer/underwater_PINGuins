use std::net::TcpStream;
use std::io::{Read, Write};
use std::io::ErrorKind;
use std::time::Instant;

use super::{
    Maurice,
    ClientSocketWrapper,
    MSG_SIZE_BYTES,
    msound_player::{ SoundEffect, },
};

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
                        let mac_str = mac.iter().map(|b| format!("{:02x}", b)).collect::<String>(); 
                        // let adc_rec_metadata = client_socket_wrapper.adc_rec_metadata.as_mut().unwrap();
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
                    // else if msg.bytes[0] & 0b11110000 == 0b01100000 { // flag for string message
                    else if msg.bytes[0] == 0b10010000 { // flag for string message
                        // append remaining 799? bytes to stream_file
                        // find last non-zero byte in msg
                        // println!("got a string message from {}", mac[5]);
                        let last_non_zero_byte = msg.bytes.iter().rposition(|&b| b != 0).unwrap();
                        // writeln!(client_socket_wrapper.stream_file.as_mut().unwrap(), "{}", String::from_utf8(msg.bytes[1..last_non_zero_byte+1].to_vec()).unwrap()).expect("failed to write to file");
                        client_socket_wrapper.fprint(&format!("{}\n",String::from_utf8(msg.bytes[1..last_non_zero_byte+1].to_vec()).unwrap()));
                    }
                } // client_socket_wrapper found
            } // match client_socket_wrapper
        }
        // sleep(self.config.max_msg_poll_interval_ms);
    }
} 