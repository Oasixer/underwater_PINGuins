use std::io::stdin;
use std::sync::mpsc::{channel, Receiver};
use std::thread;
// use std::time::Duration;
// use std::time::Instant;
// use std::io::BufWriter;
// use std::fs::File
// use std::io::Write;
use std::time::{Duration, Instant};
use std::sync::Arc;

use super::{
    Command, Maurice, AdcRecMetadata, SoundEffect, ClientSocketWrapper};
use crate::{do_nothing, config::Config};
// #[macro_use]
// use 
// use std::time::Duration;

pub fn start_stdin_listener_thread() -> Receiver<String> {
    let (tx, rx) = channel::<String>();
    thread::spawn(move || loop {
        let mut buffer = String::new();
        stdin().read_line(&mut buffer).unwrap();
        tx.send(buffer).unwrap();
    });
    rx
}

impl Maurice{
    fn report_cmd(&self, info: &str){
        println!("{}",info);
        println!("Type next command...");
    }

    fn report_cmd_fail(&self, info: &str){
        self.sound_player.play_sound_effect(SoundEffect::Error);
        self.report_cmd(info);
    }
    
    pub(super) fn print_commands_info(){
        println!("Commands:");
        println!("v <up || down>                       (increase or decrease volume)");
        println!("<xx> w <filename> <record_n_seconds>      (record to filename for n seconds)");
        println!("<xx> n <prefix>                           (select file prefix for subsequent recordings)");
        println!("<xx> w <record_n_seconds>                 (record to prefix_N for n seconds, N++ each time)");
    }

    fn handle_command(&mut self, input: String) -> (Option<Command>, Option<AdcRecMetadata>) {
        let config: Arc<Config> = self.config.clone();
        let tokens: Vec<&str> = input.trim().split_whitespace().collect();
            println!("tokens: {:?}", tokens);
        let tokens = tokens.iter().filter_map(|x| {
            if x.len() > 0 {
                Some(*x)
            } else {
                None
            }
        }).collect::<Vec<&str>>();
        
        if tokens.len() == 0 {
            self.report_cmd_fail("Invalid command length, must be 2 or 3");
            return do_nothing!();
        }
        if tokens[0].len() == 1 {
            if tokens[0] == "h" {
                Self::print_commands_info();
            }
            else{
                self.report_cmd_fail(&format!("invalid 1 token command: {}",tokens[0]));
            }
            return do_nothing!();
        }
        if tokens[0].len() == 3 {
            if tokens[0] == "v++"{
                self.sound_player.play_sound_effect(SoundEffect::IncreaseVolume);
            }
            else if tokens[0] == "v--"{
                self.sound_player.play_sound_effect(SoundEffect::ReduceVolume);
            }
            else{
                self.report_cmd_fail(&format!("invalid 1 token command: {}",tokens[0]));
            }
            return do_nothing!();
        }
        if (tokens[0].len() == 2) {
            let target_last_byte = tokens[0].as_bytes()[0];
            let config = self.config.clone();
            let target_mac = config.full_mac_or_print_available(target_last_byte);
            match target_mac {
                None => {
                   self.report_cmd_fail("");
                   return do_nothing!();
                }
                Some(mac) => {
                    let client_socket_wrapper: Result<&mut ClientSocketWrapper, String> = self.get_client_socket(mac);
                    match client_socket_wrapper {
                        Err(e) => {
                            self.report_cmd_fail(e.as_str());
                            return do_nothing!();
                        }
                        Ok(client_socket_wrapper) => {
                            let adc_rec_metadata: &mut AdcRecMetadata = &mut client_socket_wrapper.adc_rec_metadata;
                            match tokens[1]{
                                "n" => {
                                    if tokens.len() == 3{ // <xx> n <file_prefix>
                                        let file_prefix = tokens[2];
                                        adc_rec_metadata.file_prefix = Some(file_prefix.to_string());
                                        adc_rec_metadata.file_suffix = Some(0);
                                        adc_rec_metadata.file_name = None;
                                        adc_rec_metadata.duration = None;
                                        adc_rec_metadata.end_time = None;
                                    }
                                    else{
                                        self.report_cmd_fail("invalid command length for n command, must be 3");
                                        return do_nothing!();
                                    }
                                }
                                "w" => {
                                    if tokens.len() == 3 { // <xx> w <n>
                                        let record_n_seconds = tokens[2].parse::<u64>();
                                        if record_n_seconds.is_err(){
                                            self.report_cmd_fail(format!("unable to parse n={} in command with format: <xx> w <n>", tokens[3]).as_str());
                                            return do_nothing!();
                                        }
                                        let mut no_prefix: bool = false;
                                        if adc_rec_metadata.file_prefix.is_none(){
                                            no_prefix = true;
                                        }
                                        if no_prefix{
                                            self.report_cmd_fail("No file prefix set, use <xx> n <prefix> to set a file prefix before using this command.");
                                            return do_nothing!();
                                        }

                                        // adc_rec_metadata.file_suffix
                                        let duration = Duration::from_secs(record_n_seconds.unwrap());
                                        let end_time = Instant::now() + duration;
                                        adc_rec_metadata.end_time = Some(end_time);
                                        adc_rec_metadata.duration = Some(duration);
                                    }
                                    if tokens.len() == 4 { // <xx> w <filename> <n>
                                        let file_name = tokens[2];
                                        let record_n_seconds = tokens[3].parse::<u64>();
                                        if record_n_seconds.is_err(){
                                            self.report_cmd_fail(format!("unable to parse n={} in command with format: <xx> w <filename> <n>", tokens[3]).as_str());
                                            return do_nothing!();
                                        }

                                        let duration = std::time::Duration::from_secs(record_n_seconds.unwrap());
                                        let end_time = std::time::Instant::now() + duration;
                                        
                                        adc_rec_metadata.file_prefix= None;
                                        adc_rec_metadata.file_suffix= None;
                                        adc_rec_metadata.file_name= Some(String::from(file_name));
                                        adc_rec_metadata.duration= Some(duration);
                                        adc_rec_metadata.end_time= Some(std::time::Instant::now() + duration);
                                        // return (None, Some(metadata));
                                        return do_nothing!();
                                    }
                                    else{
                                        self.report_cmd_fail("invalid 2 token command: w");
                                        return do_nothing!();
                                    }
                                } // tokens[1] == "w"
                                _ => {

                                    let cmd_str: String = tokens[2..].join(" ");
                                    // fill cmd_bytes in with cmd_str, right padded w/ zeros
                                    // const msg_size_bytes: usize = self.config.OUTGOING_CMD_SIZE_BYTES;
                                    let mut cmd_bytes: [u8; super::OUTGOING_CMD_SIZE_BYTES] = [0; super::OUTGOING_CMD_SIZE_BYTES];
                                    for (i,byte) in cmd_str.bytes().enumerate(){
                                        cmd_bytes[i] = byte;
                                    }

                                    // if not a w command, then its a string command that we will allow the teensy to handle.
                                    let command = Command{
                                        target_mac: mac,
                                        bytes: cmd_bytes,
                                    };
                                    return (Some(command), None);
                                }
                            } // match tokens[1]
                        }
                    }
                } // found target mac
            } // match target mac
            // else {
            //     println!("Invalid 2 token command. Continuing.");
            // }
            
        }
        return do_nothing!();
    }

    pub(super) fn poll_for_lines_from_stdin_listener(&mut self){
        let str = self.rx_stdin_listener.try_recv();
        if str.is_err() {
            return;
        }
        match str {
            Err(_) => {},
            Ok(input) => {
                // (command: Option<Command>, adc_rec_metadata: Option<AdcRecMetadata>) = self.handle_command(input);
                let (command, adc_rec_metadata) = self.handle_command(input);
                if (command.is_some()){
                    self.tx_cmd_from_stdin_listener_to_client.send(command.unwrap()).unwrap();
                }
                if (adc_rec_metadata.is_some()){
                    let adc_rec_metadata = adc_rec_metadata.unwrap();
                    let client: Result<&mut ClientSocketWrapper, String> = self.get_client_socket(adc_rec_metadata.socket_mac);
                    match client{
                        Err(e) => {
                            println!("Error: {}", e);
                            return;
                        }
                        Ok(client) => {
                            client.adc_rec_metadata = adc_rec_metadata;
                        }
                    }
                    // self.tx_cmd_from_stdin_listener_to_client.send().unwrap();
                }
            }
        }
    }
}
