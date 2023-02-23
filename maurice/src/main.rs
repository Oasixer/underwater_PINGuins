use std::collections::HashMap;
use std::net::{TcpListener, TcpStream};
use std::{thread, any};
use std::fs::OpenOptions;
use std::io::{BufWriter, Read, Write};
use std::time::{Duration, SystemTime};
use std::io::stdin;
use std::sync::mpsc::{channel, Receiver, Sender};
use std::fs;
use std::io::BufReader;
// use rodio::{Decoder, OutputStream, Sink};
use std::fs::File;
use std::io::Cursor;
use std::io::ErrorKind;
use rodio::{Decoder, OutputStream, Sink};

// Contains needed traits
// extern crate enum_index;
// Contains derives
// #[macro_use]
// extern crate enum_index_derive;
// use enum_index::{EnumIndex, IndexEnum};

#[macro_use]
mod utils;

// #[derive(EnumIndex, IndexEnum, Debug)]
// iterable_enum!(pub(crate), SoundEffect, Connect, Disconnect, Overrun, Leak, Msg);
use strum::IntoEnumIterator;
use strum_macros::{EnumIter, Display};

const MSG_SIZE_BYTES: usize = 800;
const MSG_SOUND_EVERY_N_BYTES: u64 = 1000 * 1000;
const MSG_SOUND_EVERY_N_MSGS: u64 = MSG_SOUND_EVERY_N_BYTES / MSG_SIZE_BYTES as u64;
const TIMEOUT_READ_MS: u64 = 200;

#[derive(EnumIter, Debug, PartialEq, Eq, Display, Hash)]
enum SoundEffect {
    Connect,
    Disconnect,
    Overrun,
    Leak,
    Msg,
    StartRecording,
    EndRecording,
    ReduceVolume,
    IncreaseVolume,
}

// impl_display_for_enum!(SoundEffect);
impl SoundEffect {
    fn filename(&self) -> String {
        format!("sounds/{}.mp3", self.to_string().to_lowercase())
    }
}

// implement eq trait for SoundEffect
// impl PartialEq for SoundEffect {
//     fn eq(&self, other: &SoundEffect) -> bool {
//         self.to_string() == other.to_string()
//     }
// }

// macro_rules! impl_display_for_enum {
//     ($enum_type:ty) => {
//         impl std::fmt::Display for $enum_type {
//             fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
//                 write!(f, "{:?}", self)
//             }
//         }
//     };
// }

// macro_rules! iterable_enum {
//     ($visibility:vis, $name:ident, $($member:tt),*) => {
//         $visibility enum $name {$($member),*}
//         impl $name {
//             fn iterate_me() -> Vec<$name> {
//                 vec![$($name::$member,)*]
//             }
//         }
//     };
//     ($name:ident, $($member:tt),*) => {
//         iterable_enum!(, $name, $($member),*)
//     };
// }

const VOLUME_INCREMENT : f32 = 0.1;
const VOLUME_MAX: f32 = 1.0;
const VOLUME_MIN: f32 = 0.0;

fn main() {
    // Spawn a new thread to listen for sound commands
    let (audioplayer_tx, audioplayer_rx): (Sender<SoundEffect>, Receiver<SoundEffect>) = channel();
    thread::spawn(move || {
        let sounds_in_memory: HashMap<SoundEffect, Vec<u8>> = SoundEffect::iter()
            .map(|s| {
                let file = File::open(s.filename()).unwrap();
                let mut reader: BufReader<File> = BufReader::new(file);
                let mut buffer: Vec<u8> = Vec::new();
                reader.read_to_end(&mut buffer).unwrap();
                (s, buffer)
            })
            .collect();

        let (_stream, stream_handle) = OutputStream::try_default().unwrap();
        let sink = Sink::try_new(&stream_handle).unwrap();
        sink.set_volume(0.3);

        loop {
            match audioplayer_rx.recv() {
                Ok(sound_effect) => {
                    let sound_raw = sounds_in_memory[&sound_effect].clone();
                    let cursor = Cursor::new(sound_raw);
                    let source: Decoder<Cursor<Vec<u8>>> = rodio::Decoder::new(cursor).unwrap();                   
                    sink.append(source);
                    sink.play();

                    if (sound_effect == SoundEffect::IncreaseVolume){
                        let new_volume = sink.volume() + VOLUME_INCREMENT;
                        if new_volume <= VOLUME_MAX {
                            sink.set_volume(new_volume);
                        }
                    } else if (sound_effect == SoundEffect::ReduceVolume){
                        let new_volume = sink.volume() - VOLUME_INCREMENT;
                        if new_volume >= VOLUME_MIN {
                            sink.set_volume(new_volume);
                        }
                    }
                }
                Err(_) => { println!("Error receiving sound command");}
            }
        }
    });

    // let host = "192.168.1.70";
    let host = fs::read_to_string("./this_host_ip")
        .expect("Failed to read file")
        .trim()
        .to_owned();
    let port = 6969;
    let mut buffer = [0u8; 1024];
    let mut conn: Option<std::net::TcpStream> = None;
    let mut file: Option<BufWriter<std::fs::File>> = None;
    let mut end_time: Option<SystemTime> = None;
    let mut file_path: std::string::String = String::new();
    let mut file_bytes_written: u64 = 0;
    let mut n_secs = 1;
    let mut msg_read_num: u64 = 0;
    let mut volume: f32 = 0.6;

    let stdin_channel = spawn_stdin_channel();
    let adc_rec_dir_path = "../adc_recordings";
    if let Err(e) = fs::metadata(&adc_rec_dir_path) {
        if e.kind() == std::io::ErrorKind::NotFound {
            // The directory does not exist, so create it
            match fs::create_dir(&adc_rec_dir_path) {
                Ok(()) => println!("Directory created: {}", adc_rec_dir_path),
                Err(e) => eprintln!("Error creating directory: {}", e),
            }
        } else {
            println!("Error checking directory: {}", e);
        }
    } else {
        // The directory exists
        println!("Found adc_recordings dir: {}", adc_rec_dir_path);
    }

    loop {
        match stdin_channel.try_recv() {
            Ok(input) => {
                let tokens: Vec<&str> = input.trim().split_whitespace().collect();
                // println!("received input: {}, len {}", input.trim(), tokens.len());
                let mut skip: bool = false;
                match tokens.len() {
                    1 => {
                        println!("invalid command length, must be 2 or 3");
                        continue;
                    }
                    2 => {
                        file_path = match tokens[0] {
                            "n" => {
                                skip = true;
                                println!("\nSet prefix to {}", tokens[1]);
                                format!("{}/{}_0", adc_rec_dir_path, String::from(tokens[1]))
                            },
                            "w" => {
                                // check if file_path ends in _N where N is a number, store n
                                // if so, increment n and append to file_path
                                match file_path.rfind("_") {
                                    Some(n) => {
                                        let mut split: Vec<&str> = file_path.split("_").collect();
                                        let new_len = split.len() - 1;
                                        let mut n_str: String = String::new();
                                        for final_element_only_one in split.drain(new_len..){
                                            n_str = final_element_only_one.to_string();
                                        }
                                        let prefix: String = split.join("_");
                                        let n = n_str.parse::<u32>().expect("file ended in _xxx but xxx was not a number");
                                        n_secs = tokens[1].parse().expect("w command <seconds> token was not a number");
                                        format!("{}_{}",prefix, n+1)
                                    }
                                    None => panic!("2 tokens in 'w' command, yet 'n' command not previously issued")
                                }
                            }
                            "v" => {
                                if tokens[1] == "up" {
                                    audioplayer_tx.send(SoundEffect::IncreaseVolume).unwrap();
                                }
                                else if tokens[1] == "down"{
                                    audioplayer_tx.send(SoundEffect::ReduceVolume).unwrap();
                                }
                                file_path // keep old file_path
                            }
                            _ => panic!("invalid token[0]: {}", tokens[0])
                        }
                    }
                    3 => {
                        if tokens[0] != "w" {
                            println!("invalid command: {}", tokens[0]);
                        }
                        else{
                            file_path = format!("{}/{}", adc_rec_dir_path, String::from(tokens[1]));
                            n_secs = tokens[2].parse().expect("w command <seconds> token was not a number");
                        }
                    }
                    _ => {println!("invalid command length, must be 2 or 3")}
                }
                if !skip && file_path.len() > 0 {
                    // Open file for writing
                    if let Ok(f) = OpenOptions::new().create(true).write(true).open(&file_path) {
                        file = Some(BufWriter::new(f));
                    } else {
                        panic!("Failed to open file!");
                    }
                    println!("\nBegin recording to file {} for {} seconds", file_path, n_secs);
                    audioplayer_tx.send(SoundEffect::StartRecording).unwrap();
                    end_time = Some(SystemTime::now() + Duration::from_secs(n_secs));
                }
            },
            Err(_) => {},
        }

        // Check if end_time has passed
        if let Some(et) = end_time {
            if SystemTime::now() > et {
                end_time = None;
                // let file_metadata = file.as_ref().unwrap().get_ref().metadata().unwrap();
                println!("Finished recording to {}, wrote {} bytes @ {} [kB/s]", file_path, file_bytes_written, file_bytes_written as f64 / n_secs as f64 / 1000.0);
                file = None;
                file_bytes_written = 0;
                audioplayer_tx.send(SoundEffect::EndRecording).unwrap();
            }
        }

        // If conn is not yet set, try to connect
        if conn.is_none() {
            match std::net::TcpListener::bind((host.clone(), port)) {
                Ok(listener) => {
                    println!("\nLISTENING ON {}... [   BLOCKING   ]... WAITING FOR CLIENT", host);
                    // listener.set_nonblocking(true).expect("Failed to initialize non-blocking.");
                    
                    for stream in listener.incoming() {
                        match stream {
                            Ok(c) => {
                                conn = Some(c);
                                println!("");
                                println!("=============== PINGUIN CONNECTED {} ================", conn.as_ref().unwrap().peer_addr().unwrap());
                                println!("Commands:");
                                println!("v <up || down>                       (increase or decrease volume)");
                                println!("w <filename> <record_n_seconds>      (record to filename for n seconds)");
                                println!("n <prefix>                           (select file prefix for subsequent recordings)");
                                println!("w <record_n_seconds>                 (record to prefix_N for n seconds, N++ each time)");
                                audioplayer_tx.send(SoundEffect::Connect).unwrap();
                                conn.as_ref().unwrap().set_read_timeout(Some(Duration::from_millis(TIMEOUT_READ_MS))).expect("Failed to set read timeout.");
                                break;
                            },
                            Err(e) => {
                                // if not connected yet for the first time, error is expected
                                // error: A non-blocking socket operation could not be completed immediately. (os error 10035)
                                println!("error: {}", e);
                            }
                        }
                    }
                },
                Err(e) => {
                    println!("Exception: {}", e);
                    std::thread::sleep(Duration::from_secs(1));
                }
            }
        }

        // If conn is set, try to read from it
        if let Some(ref mut c) = conn {
            match c.read(&mut buffer) {
                Ok(0) => {
                    println!("Connection closed!");
                    conn = None;
                },
                Ok(size) => {
                    let message_count = size / MSG_SIZE_BYTES as usize;
                    // if (message_count > 1){
                    // println!("msg count: {}", message_count);
                    // }
                    for i in 0..message_count {
                        let start = i * MSG_SIZE_BYTES;
                        let end = start + MSG_SIZE_BYTES;
                        let message = &buffer[start..end];
                        msg_read_num += 1;
                        if (msg_read_num % MSG_SOUND_EVERY_N_MSGS == 0) {
                            audioplayer_tx.send(SoundEffect::Msg).unwrap();
                        }
                        // check first 4 bits of first byte in message.
                        // if first 4 bits are 1111, then panic as a leak was detected
                        // if first 4 bits are 0101, then the buffer was overran, so print a warning

                        // println!("msg[0]: {}", message[0]>>4);
                        // println!("msg[1]: {}", message[1]);
                        match message[0] >> 4 {
                            0b1111 => {
                                // print first 5 bytes of message
                                println!("Leak detected!");
                                for i in 0..100 {
                                    println!("byte{}: {:02x}, >>4: {}", i,  message[i], message[i] >> 4);
                                }
                                println!("LEAK DETECTED!!!!!!!!!!");
                                audioplayer_tx.send(SoundEffect::Leak).unwrap();

                                panic!("Leak detected!");
                                // println!("msg: {}", message);
                            }
                            0b0101 => {
                                // println!("Buffer overrun detected!");
                                audioplayer_tx.send(SoundEffect::Overrun).unwrap();
                            }
                            _ => {}
                        }

                        if let Some(ref mut f) = file {
                            file_bytes_written += message.len() as u64;
                            f.write_all(message).unwrap();
                        }
                    }
                },
                Err(e) => {
                    match e.kind() {
                        ErrorKind::TimedOut => {
                            println!("\nPINGUIN @{} DISCONNECTED  [ TIMED OUT AFTER {}ms ]\n   :'(\n", conn.as_ref().unwrap().peer_addr().as_ref().unwrap(), TIMEOUT_READ_MS);
                        },
                        _ => {
                            println!("PINGUIN @{} DISCONNECTED (due to ERROR={})\n     :'(\n", conn.as_ref().unwrap().peer_addr().as_ref().unwrap(), e);
                        } 
                    }

                    audioplayer_tx.send(SoundEffect::Disconnect).unwrap();
                    conn = None;
                }
            }
        }
    }
}

fn spawn_stdin_channel() -> Receiver<String> {
    let (tx, rx) = channel::<String>();
    thread::spawn(move || loop {
        let mut buffer = String::new();
        stdin().read_line(&mut buffer).unwrap();
        tx.send(buffer).unwrap();
    });
    rx
}

// fn sleep(millis: u64) {
//     let duration = Duration::from_millis(millis);
//     thread::sleep(duration);
// }