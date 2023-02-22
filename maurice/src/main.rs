use std::net::{TcpListener, TcpStream};
use std::thread;
use std::fs::OpenOptions;
use std::io::{BufWriter, Read, Write};
use std::time::{Duration, SystemTime};
use std::io::stdin;
use std::sync::mpsc::{channel, Receiver};
use std::fs;


fn main() {
    // let host = "192.168.1.70";
    let host = fs::read_to_string("./this_host_ip")
        .expect("Failed to read file")
        .trim()
        .to_owned();
    let port = 6969;
    let message_size = 800;
    let mut buffer = [0u8; 1024];
    let mut conn: Option<std::net::TcpStream> = None;
    let mut file: Option<BufWriter<std::fs::File>> = None;
    let mut end_time: Option<SystemTime> = None;
    let mut file_path = String::new();
    let mut n_secs = 1;

    let stdin_channel = spawn_stdin_channel();

    loop {
        match stdin_channel.try_recv() {
            Ok(input) => {
                let tokens: Vec<&str> = input.trim().split_whitespace().collect();
                if tokens.len() == 3 && tokens[0] == "write_me" {
                    file_path = String::from(tokens[1]);
                    n_secs = tokens[2].parse().unwrap();
                    if let Ok(f) = OpenOptions::new().create(true).write(true).open(&file_path) {
                        file = Some(BufWriter::new(f));
                    } else {
                        panic!("Failed to open file!");
                    }
                    end_time = Some(SystemTime::now() + Duration::from_secs(n_secs));
                }
            },
            Err(_) => {},
        }

        // Check if end_time has passed
        if let Some(et) = end_time {
            if SystemTime::now() > et {
                end_time = None;
                file = None;
                file_path = String::new();
            }
        }

        // If conn is not yet set, try to connect
        if conn.is_none() {
            match std::net::TcpListener::bind((host.clone(), port)) {
                Ok(listener) => {
                    println!("listening");
                    for stream in listener.incoming() {
                        match stream {
                            Ok(c) => {
                                conn = Some(c);
                                println!("Received connect from {}", conn.as_ref().unwrap().peer_addr().unwrap());
                                break;
                            },
                            Err(e) => {
                                panic!("error: {}", e);
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
                    let message_count = size / message_size;
                    // if (message_count > 1){
                    println!("msg count: {}", message_count);
                    // }
                    for i in 0..message_count {
                        let start = i * message_size;
                        let end = start + message_size;
                        let message = &buffer[start..end];
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
                                panic!("Leak detected!");
                                // println!("msg: {}", message);
                            }
                            0b0101 => println!("Buffer overrun detected!"),
                            _ => {}
                        }

                        if let Some(ref mut f) = file {
                            f.write_all(message).unwrap();
                        }
                    }
                },
                Err(e) => {
                    match e.kind() {
                        std::io::ErrorKind::WouldBlock => {
                            // There is no data to read
                        },
                        _ => {
                            panic!("error: {}", e);
                        }
                    }
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

fn sleep(millis: u64) {
    let duration = Duration::from_millis(millis);
    thread::sleep(duration);
}