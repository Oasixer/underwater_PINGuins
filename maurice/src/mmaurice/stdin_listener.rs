use std::io::stdin;
use std::sync::mpsc::{channel, Receiver};
use std::thread;
use std::io::Write;
use std::time::{Duration, Instant};

use super::{
    Command, Maurice, AdcRecMetadata, SoundEffect, ClientSocketWrapper, ConnectionChange};
use crate::{do_nothing, config::Config, utils::hex_string_to_u8};
use crate::config::{DATA_STREAM_DIR, ADC_DATA_DIR};
use crate::utils::{create_directories, thread_sleep};
use rustyline::error::ReadlineError;
use rustyline;
// use rustyline::{DefaultEditor, Result};

// #[cfg(feature = "with-file-history")]
// #[macro_use]
// use 
// use std::time::Duration;

// pub fn start_stdin_listener_thread() -> Receiver<String> {
//     let (tx, rx) = channel::<String>();
//     thread::spawn(move || loop {
        
//         let mut buffer = String::new();
//         stdin().read_line(&mut buffer).unwrap();
//         if buffer.len() > 0 {
//             tx.send(buffer).unwrap();
//         }
//     });
//     rx
// }

pub fn start_stdin_listener_thread2() -> Receiver<String> {
    let (tx, rx) = channel::<String>();
    thread::spawn(move || {
        let mut rl = rustyline::DefaultEditor::new().unwrap();

        #[cfg(feature = "with-file-history")]
        if rl.load_history("history.txt").is_err() {
            println!("No previous history.");
        }
        loop {
            // `()` can be used when no completer is required
            let mut buffer = String::new();
            let readline = rl.readline(">> ");
            match readline {
                Ok(line) => {
                    rl.add_history_entry(line.as_str());
                    buffer = line;
                    // println!("Line: {}", &buffer);
                    rl.save_history("history.txt");
                },
                Err(ReadlineError::Interrupted) => {
                    println!("CTRL-C");
                    break
                },
                Err(ReadlineError::Eof) => {
                    println!("CTRL-D");
                    break
                },
                Err(err) => {
                    println!("Error: {:?}", err);
                    break
                }
            }
            if buffer.len() > 0 {
                tx.send(buffer).unwrap();
            }
            thread_sleep(200);
        }
    });
    rx
        // Ok(())
            // stdin().read_line(&mut buffer).unwrap();
}

impl Maurice{
    pub fn report_cmd(&self, info: &str){
        println!("{}",info);
        // println!("Type next command...");
    }

    fn report_cmd_fail(&self, info: &str){
        self.sound_player.play_sound_effect(SoundEffect::Error);
        self.report_cmd(info);
    }
    
    pub(super) fn print_commands_info(){
        // println!("Commands:");
        // println!("v <up || down>                            (increase or decrease volume)");
        // println!("<xx> w <filename> <record_n_seconds>      (record to filename for n seconds)");
        // println!("<xx> n <prefix>                           (select file prefix for subsequent recordings)");
        // println!("<xx> w <record_n_seconds>                 (record to prefix_N for n seconds, N++ each time)");
    }

    fn handle_command_for_client(&mut self, target_mac_byte: &str, tokens: Vec<&str>) -> (Option<Command>, bool){
        // println!("Handle command for client");
        if (target_mac_byte == "xx"){ // TODO handle this nicer, this is duplicate code from the
            // normal command sending section below.
            let cmd_str: String = tokens.join(" ");
            let mut cmd_bytes: [u8; super::OUTGOING_CMD_SIZE_BYTES] = [0; super::OUTGOING_CMD_SIZE_BYTES];
            for (i,byte) in cmd_str.bytes().enumerate(){
                cmd_bytes[i] = byte;
            }

            // if not a w command, then its a string command that we will allow the teensy to handle.
            let command = Command{
                target_mac: [0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
                bytes: cmd_bytes,
            };
            self.report_cmd(&format!("Sending command: <{}> to ALL",cmd_str));
            return (Some(command), true);
        }

        let target_last_byte = hex_string_to_u8(target_mac_byte);
        if target_last_byte.is_err(){
            self.report_cmd_fail(&format!("invalid hex string: {}",target_mac_byte));
            return (None, false);
        }
        let target_last_byte = target_last_byte.unwrap();

        let config = self.config.clone();
        let target_mac = config.full_mac_or_print_available(target_last_byte);
        match target_mac {
            None => {
                self.report_cmd_fail("");
                return (None, false);
            }
            Some(mac) => {
                let client_socket_wrapper: Result<&mut ClientSocketWrapper, String> = self.get_client_socket(mac);

                match client_socket_wrapper {
                    Err(e) => {
                        self.report_cmd_fail(e.as_str());
                        return (None, false);
                    }
                    Ok(client_socket_wrapper) => {
                        let adc_rec_metadata: &mut AdcRecMetadata = &mut client_socket_wrapper.adc_rec_metadata;
                        let adc_dir_path = format!("{}/{}/{}", DATA_STREAM_DIR, mac.iter().map(|b| format!("{:02x}", b)).collect::<String>(), ADC_DATA_DIR);
                        create_directories(&adc_dir_path).unwrap();
                        match tokens[0]{
                            "n" => {
                                if tokens.len() == 2{ // <xx> n <file_prefix>
                                    let file_prefix = tokens[1];
                                    adc_rec_metadata.file_prefix = Some(format!("{}/{}",adc_dir_path, file_prefix.to_string()));
                                    adc_rec_metadata.file_suffix = Some(0);
                                    adc_rec_metadata.file_name = None;
                                    adc_rec_metadata.duration = None;
                                    adc_rec_metadata.end_time = None;
                                    // client_socket_wrapper.report_begin_recording();
                                    client_socket_wrapper.fprint(&format!("Set file prefix to: {}\n", file_prefix));
                                    return (None, false);
                                }
                                else{
                                    self.report_cmd_fail("invalid command length for n command, must be 3");
                                    return (None, false);
                                }
                            }
                            "w" => {
                                if tokens.len() == 2 { // <xx> w <n>
                                    let record_n_seconds = tokens[1].parse::<u64>();
                                    if record_n_seconds.is_err(){
                                        self.report_cmd_fail(format!("unable to parse n={} in command with format: <xx> w <n>", tokens[3]).as_str());
                                        return (None, false);
                                    }
                                    let mut no_prefix: bool = false;
                                    if adc_rec_metadata.file_prefix.is_none(){
                                        no_prefix = true;
                                    }
                                    if no_prefix{
                                        self.report_cmd_fail("No file prefix set, use <xx> n <prefix> to set a file prefix before using this command.");
                                        return (None, false);
                                    }

                                    // adc_rec_metadata.file_suffix
                                    let duration = Duration::from_secs(record_n_seconds.unwrap());
                                    let end_time = Instant::now() + duration;
                                    adc_rec_metadata.end_time = Some(end_time);
                                    adc_rec_metadata.duration = Some(duration);
                                    // self.sound_player.play_sound_effect(SoundEffect::Recording);
                                    // let player = self.sound_player.audioplayer_tx.clone();
                                    // player.send(SoundEffect::StartRecording);
                                    client_socket_wrapper.report_begin_recording();
                                    return (None, true);
                                }
                                if tokens.len() == 3 { // <xx> w <filename> <n>
                                    let file_name = tokens[1];
                                    let record_n_seconds = tokens[2].parse::<u64>();
                                    if record_n_seconds.is_err(){
                                        self.report_cmd_fail(format!("unable to parse n={} in command with format: <xx> w <filename> <n>", tokens[2]).as_str());
                                        return (None, false);
                                    }

                                    let duration = std::time::Duration::from_secs(record_n_seconds.unwrap());
                                    let end_time = std::time::Instant::now() + duration;
                                    
                                    adc_rec_metadata.file_prefix= None;
                                    adc_rec_metadata.file_suffix= None;
                                    adc_rec_metadata.file_name= Some(format!("{}/{}", adc_dir_path, String::from(file_name)));
                                    adc_rec_metadata.duration= Some(duration);
                                    adc_rec_metadata.end_time= Some(end_time);
                                    client_socket_wrapper.report_begin_recording();
                                    return (None, true);
                                }
                                else{
                                    self.report_cmd_fail("invalid 2 token command: w");
                                    return (None, false);
                                }
                            }
                            _ => {
                                let cmd_str: String = tokens.join(" ");
                                // fill cmd_bytes in with cmd_str, right padded w/ zeros
                                // const msg_size_bytes: usize = self.config.OUTGOING_CMD_SIZE_BYTES;
                                let mut cmd_bytes: [u8; super::OUTGOING_CMD_SIZE_BYTES] = [0; super::OUTGOING_CMD_SIZE_BYTES];
                                for (i,byte) in cmd_str.bytes().enumerate(){
                                    cmd_bytes[i] = byte;
                                }

                                // if not a w command, then its a string command that we will allow the teensy to handle.
                                let mac_str = format!("{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                                let command = Command{
                                    target_mac: mac,
                                    bytes: cmd_bytes,
                                };
                                self.report_cmd(&format!("Sending command: <{}> to {}",cmd_str, mac_str));
                                return (Some(command), false);
                            }
                        }
                    }
                }
            }
        }
    }

    fn handle_command(&mut self, input: String) -> Option<Command> {
        // let config: Config = self.config.clone();
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
            return None;
        }
        if tokens.len() == 1 {
            if tokens[0] == "h" {
                Self::print_commands_info();
            }
            else{
                self.report_cmd_fail(&format!("invalid 1 token command: {}",tokens[0]));
            }
            return None;
        }
        if tokens[0].len() == 3 {
            if tokens[0] == "v++"{
                self.report_cmd("Increasing volume");
                self.sound_player.play_sound_effect(SoundEffect::IncreaseVolume);
            }
            else if tokens[0] == "v--"{
                self.report_cmd("Decreasing volume.");
                self.sound_player.play_sound_effect(SoundEffect::ReduceVolume);
            }
            else if tokens[0].chars().nth(2).unwrap() == '/'{
            }
            else{
                self.report_cmd_fail(&format!("invalid command starting w/ a 3-char token: {}", input));
                return None;
            }
        }
        if tokens[0].len() == 2{
            let tokens_1_onwards_vec = tokens[1..].to_vec();
            let (result_cmd, play_sound) = self.handle_command_for_client(&tokens[0], tokens_1_onwards_vec);
            if play_sound{
                self.sound_player.play_sound_effect(SoundEffect::StartRecording);
            }

            if result_cmd.is_some(){
                let result_cmd = result_cmd.unwrap();
                // let target_mac = result_cmd.target_mac;
                return Some(result_cmd);
                // let client: &mut ClientSocketWrapper = self.get_client_socket(target_mac).expect("unable to get client socket");
                // client.send_command(result_cmd);
                // client.fprint("Sending command: {}", input);
            }
        }
        else{
            // self.sound_player.play_sound_effect(SoundEffect::Error);
            self.report_cmd_fail(&format!("invalid command: {}", input));
            return None;
        }
        // }
                // } // found target mac
            // } // match target mac
            // else {
            //     println!("Invalid 2 token command. Continuing.");
            // }
            
        return None;
    }

    pub(super) fn remove_client(&mut self, mac: [u8; 6]){
        let mut index: Option<usize> = None;
        for (i, client) in self.client_sockets.iter().enumerate(){
            if client.mac == mac{
                index = Some(i);
                break;
            }
        }
        if index.is_some(){
            let index = index.unwrap();
            self.client_sockets.remove(index);
        }
    }

    pub(super) fn poll_for_lines_from_stdin_listener(&mut self){
        let str = self.rx_stdin_listener.try_recv();
        if str.is_err() {
            return;
        }
        match str {
            Err(_) => {},
            Ok(input) => {
                if input.len() == 0 {
                    return;
                }
                // (command: Option<Command>, adc_rec_metadata: Option<AdcRecMetadata>) = self.handle_command(input);
                let command = self.handle_command(input);
                if command.is_some(){
                    let command = command.unwrap();
                    let command_str = String::from_utf8(command.bytes.to_vec()).unwrap();
                    let target_mac = command.target_mac;
                    if target_mac == [0,0,0,0,0,0]{ // send to all
                    // clients = self.client_sockets
                    //     .into_iter()
                    //     .filter_map(|mut client| {
                    //         let mut buff = msg.clone().into_bytes();
                    //         buff.resize(MSG_SIZE, 0);

                    //         client.write_all(&buff).map(|_| client).ok()
                    //     })
                    // .collect::<Vec<_>>();
                        for client in self.client_sockets.iter_mut(){
                            client.fprint(&format!("Sending command: {}\n", command_str));
                            let result = client.stream.write_all(&command.bytes);
                            if result.is_err(){
                                println!("Client stopped receiving commands. Removing client.");
                                client.latest_connection_change = Some(ConnectionChange{
                                    mac: client.mac,
                                    is_connected: false,
                                });
                                // self.remove_client(client.mac); 
                            }
                        }
                        return;
                    }
                    let client: &mut ClientSocketWrapper = self.get_client_socket(target_mac).expect("unable to get client socket");
                    // read utf8 string from command.bytes:
                    client.fprint(&format!("Sending command: {}\n", command_str));
                    client.stream.write_all(&command.bytes).expect("unable to send command");
                    // client.tx_cmd_from_stdin_listener_to_client.send(command).unwrap();
                }
                // if (adc_rec_metadata.is_some()){
                //     let adc_rec_metadata = adc_rec_metadata.unwrap();
                //     let client: Result<&mut ClientSocketWrapper, String> = self.get_client_socket(adc_rec_metadata.socket_mac);
                //     match client{
                //         Err(e) => {
                //             println!("Error: {}", e);
                //             return;
                //         }
                //         Ok(client) => {
                //             client.adc_rec_metadata = adc_rec_metadata;
                //         }
                //     }
                //     // self.tx_cmd_from_stdin_listener_to_client.send().unwrap();
                // }
            }
        }
    }
}
