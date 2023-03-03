
use std::thread;
// use std::io::{Read, ErrorKind};
// use chrono::prelude::*;
use std::fs::File;
use std::io::Write;
use std::sync::mpsc::{channel, Sender, Receiver};
use std::time::Instant;

// use rocket::{get, routes};
// use rocket_contrib::json::Json;


use super::{
    Maurice,
    Coord3D,
    // DisplayData,
};

// fn display_data() -> impl Serialize {
// #[get("/todo")]
// fn display_data() -> Json<Coord3D> {
//     let coords = Coord3D { x: 1.0, y: 2.0, z: 3.0 };
//     // let updated = Instant::now();
//     Json(coords)
// }

// // impl Maurice {
// pub fn start_web_server_thread() -> Sender<Coord3D>{
//     // let server = tiny_http::Server::http("192.168.1.70:6970").unwrap();
//     let rocket = rocket::ignite().mount("/", routes![display_data]);
//     let (tx_display_data, rx_display_data) = channel::<DisplayData>();
//         // let handle = std::thread::spawn(move || {
//         //     rocket.launch();
//         // });
//     return tx_display_data;
// }
// // }

// #[macro_use] extern crate rocket;

// #[cfg(test)] mod tests;

use rocket::{State, Config};
use rocket::fairing::AdHoc;
use rocket::serde::Deserialize;

#[derive(Debug, Deserialize)]
#[serde(crate = "rocket::serde")]
#[allow(dead_code)]
struct AppConfig {
    key: String,
    port: u16
}

#[get("/")]
fn read_config(rocket_config: &Config, app_config: &State<AppConfig>) -> String {
    format!("{:#?}\n{:#?}", app_config, rocket_config)
}

// See Rocket.toml file. Running this server will print the config. Try running
// with `ROCKET_PROFILE=release` manually by setting the environment variable
// and automatically by compiling with `--release`.
#[launch]
pub fn rocket() -> _ {
    rocket::build()
        .mount("/", routes![read_config])
        .attach(AdHoc::config::<AppConfig>())
}


pub struct SoundPlayer {
    // sounds_in_memory: HashMap<SoundEffect, Vec<u8>>,
    // sink: Sink,
    // pub audioplayer_tx: Sender<SoundEffect>,
    // audioplayer_rx: Receiver<SoundEffect>,
}