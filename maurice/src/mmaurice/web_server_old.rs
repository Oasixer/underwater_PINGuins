use std::fs::File;
use std::io::Write;
use std::sync::mpsc::{channel, Sender, Receiver};
use std::time::Instant;
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

use rocket::{State, Config};
// use rocket::fairing::AdHoc;
use rocket::serde::Deserialize;
use rocket::{get, routes};
use rocket::{Request};
use rocket::request::{FromRequest, Outcome};

use super::{
    Maurice,
    Coord3D,
    // DisplayData,
};

struct MyStruct {
    my_variable: String,
}


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

// #[derive(Debug, Deserialize)]
// #[serde(crate = "rocket::serde")]
// #[allow(dead_code)]
// struct AppConfig {
//     key: String,
//     port: u16
// }

// #[get("/")]
// fn read_config(rocket_config: &Config, app_config: &State<AppConfig>) -> String {
//     format!("{:#?}\n{:#?}", app_config, rocket_config)
// }



// See Rocket.toml file. Running this server will print the config. Try running
// with `ROCKET_PROFILE=release` manually by setting the environment variable
// and automatically by compiling with `--release`.
// #[launch]
// pub fn rocket() -> _ {
//     let (webserver_data_tx, webserver_data_rx): (Sender<String>, Receiver<String>) = channel();
//     // loop {
//     //     // match webserver_data_rx.recv() {
//     //     // }
//     // }

//     thread::spawn(move || {
//     rocket::build()
//         .mount("/", routes![read_config])
//         .attach(AdHoc::config::<AppConfig>())
//     });
// }
        // let sounds_in_memory: HashMap<SoundEffect, Vec<u8>> = SoundEffect::iter()
            // .map(|s| {
            //     let file = File::open(s.filename()).unwrap();
            //     let mut reader: BufReader<File> = BufReader::new(file);
            //     let mut buffer: Vec<u8> = Vec::new();
            //     reader.read_to_end(&mut buffer).unwrap();
            //     (s, buffer)
            // })
            // .collect();

        // let (_stream, stream_handle) = OutputStream::try_default().unwrap();
        // let sink = Sink::try_new(&stream_handle).unwrap();
        // sink.set_volume(0.3);

    //     loop {
    //         match audioplayer_rx.recv() {
    //             Ok(sound_effect) => {
    //                 let sound_raw = sounds_in_memory[&sound_effect].clone();
    //                 let cursor = Cursor::new(sound_raw);
    //                 let source: Decoder<Cursor<Vec<u8>>> = rodio::Decoder::new(cursor).unwrap();                   
    //                 sink.append(source);
    //                 sink.play();

    //                 sink.sleep_until_end();

    //                 if (sound_effect == SoundEffect::IncreaseVolume){
    //                     let new_volume = sink.volume() + VOLUME_INCREMENT;
    //                     if new_volume <= VOLUME_MAX {
    //                         sink.set_volume(new_volume);
    //                     }
    //                 } else if (sound_effect == SoundEffect::ReduceVolume){
    //                     let new_volume = sink.volume() - VOLUME_INCREMENT;
    //                     if new_volume >= VOLUME_MIN {
    //                         sink.set_volume(new_volume);
    //                     }
    //                 }
    //             }
    //             Err(_) => { println!("Error receiving sound command");}
    //         }
    //     }
    // });


// pub struct SoundPlayer {
    // sounds_in_memory: HashMap<SoundEffect, Vec<u8>>,
    // sink: Sink,
    // pub audioplayer_tx: Sender<SoundEffect>,
    // audioplayer_rx: Receiver<SoundEffect>,
// }


// use std::sync::{Arc, Mutex};
// use std::thread;
// use std::time::Duration;

// struct MyStruct {
//     my_variable: String,
// }

struct MyStructGuard(Arc<Mutex<MyStruct>>);

impl<'r> FromRequest<'r> for MyStructGuard {
    type Error = ();

    fn from_request(request: &'r Request<'r>) -> Outcome<Self, Self::Error> {
        match request.guard::<&rocket::State<Arc<Mutex<MyStruct>>>>() {
            rocket::Outcome::Success(my_struct) => rocket::Outcome::Success(MyStructGuard(my_struct.inner().clone())),
            rocket::Outcome::Failure(e) => rocket::Outcome::Failure(e),
            rocket::Outcome::Forward(_) => rocket::Outcome::Forward(()),
        }
    }
}

#[get("/")]
fn index(my_struct: MyStructGuard) -> String {
    let my_struct = my_struct.0.lock().unwrap();
    format!("The value of my_variable is: {}", my_struct.my_variable)
}

fn main() {
    let my_struct = MyStruct {
        my_variable: "Hello, world!".to_string(),
    };

    let my_struct = Arc::new(Mutex::new(my_struct));

    let my_struct_clone = my_struct.clone();
    thread::spawn(move || {
        rocket::ignite()
            .manage(my_struct_clone)
            .mount("/", routes![index])
            .launch();
    });

    loop {
        {
            let mut my_struct = my_struct.lock().unwrap();
            my_struct.my_variable = "Hello, universe!".to_string();
        }
        thread::sleep(Duration::from_secs(1));
    }
}
