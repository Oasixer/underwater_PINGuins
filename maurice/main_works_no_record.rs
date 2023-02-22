use std::net::{TcpListener, TcpStream};
use std::io::{Read};
use std::time::{Duration, Instant};
use std::thread;
use std::process;

fn main() {
    let host = "192.168.1.70";
    let port = 6969;
    let measure_speed_every_n_packets = Some(100);

    loop {
        match TcpListener::bind((host, port)) {
            Ok(listener) => {
                println!("listening");
                for stream in listener.incoming() {
                    match stream {
                        Ok(mut conn) => {
                            let addr = conn.peer_addr().unwrap();
                            println!("Connected by {}", addr);
                            let mut n = 0;
                            let mut speed_measure_bytes = 0;
                            let mut begin_speed_measure_ts = Instant::now();
                            loop {
                                let mut buf = [0u8; 10000];
                                match conn.read(&mut buf) {
                                    Ok(size) => {
                                        if size == 0 {
                                            // Client has disconnected
                                            println!("Client {} has disconnected", addr);
                                            break;
                                        }
                                        if let Some(n_packets) = measure_speed_every_n_packets {
                                            speed_measure_bytes += size;
                                            if n > 0 && n % n_packets == 0 {
                                                let cur = Instant::now();
                                                let elapsed = cur.duration_since(begin_speed_measure_ts);
                                                let rate = (speed_measure_bytes as f64) / (elapsed.as_secs_f64() * 1000.0);
                                                println!("rate({}): {} kB/s", n_packets, rate);
                                                begin_speed_measure_ts = Instant::now();
                                                speed_measure_bytes = 0;
                                            }
                                        }
                                        n += 1;
                                        println!("n: {}", n);
                                    },
                                    Err(e) => {
                                        match e.kind() {
                                            std::io::ErrorKind::ConnectionReset => {
                                                // Client has disconnected
                                                println!("Client {} has disconnected", addr);
                                                break;
                                            },
                                            _ => {
                                                panic!("error: {}", e);
                                            }
                                        }
                                    }
                                }
                            }
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
}
