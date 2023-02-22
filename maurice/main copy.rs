use std::fs::OpenOptions;
use std::io::{BufWriter, Read, Write};
use std::net::{TcpListener, TcpStream};
use std::thread;
use std::time::{Duration, SystemTime};

fn main() {
    let host = "192.168.1.70";
    let port = 6969;
    // let message_size = 200;
    let message_size = 800;
    let mut buffer = [0u8; 1024];
    let mut conn: Option<TcpStream> = None;
    let mut file: Option<BufWriter<std::fs::File>> = None;
    let mut end_time: Option<SystemTime> = None;
    let n_secs = 1;
    let file_path = "./write_me";

    loop {
        // Check if the file exists
        if let Ok(_) = std::fs::metadata(file_path) {
            let size = std::fs::metadata(file_path).unwrap().len();
            if size == 0 {
                if end_time.is_none() {
                    // If the file exists but end_time is not set, set it
                    end_time = Some(SystemTime::now() + Duration::from_secs(n_secs));
                    if let Ok(f) = OpenOptions::new().append(true).open("./write_me") {
                        file = Some(BufWriter::new(f));
                    } else {
                        panic!("Failed to open file!");
                    }
                }
            }
        } else {
            // If the file doesn't exist, reset the end_time and file
            end_time = None;
            file = None;
        }

        // Check if end_time has passed
        if let Some(et) = end_time {
            if SystemTime::now() > et {
                end_time = None;
                file = None;
            }
        }

        // If conn is not yet set, try to connect
        if conn.is_none() {
            match TcpListener::bind((host, port)) {
                Ok(listener) => {
                    println!("listening");
                    for stream in listener.incoming() {
                        match stream {
                            Ok(c) => {
                                conn = Some(c);
                                println!("Connected by {}", conn.as_ref().unwrap().peer_addr().unwrap());
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
                    thread::sleep(Duration::from_secs(1));
                }
            }
        }

        // If conn is set, try to read from it
        if let Some(ref mut c) = conn {
            match c.read(&mut buffer) {
                Ok(0) => {
                    // The connection was closed
                    println!("Connection closed!");
                    conn = None;
                },
                Ok(size) => {
                    let message_count = size / message_size;
                    if (message_count > 1){
                        println!("msg count: {}", message_count);
                    }
                    for i in 0..message_count {
                        let start = i * message_size;
                        let end = start + message_size;
                        let message = &buffer[start..end];
                        if let Some(ref mut f) = file {
                            f.write_all(message).unwrap();
                            // f.write_all(b"\n").unwrap();
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