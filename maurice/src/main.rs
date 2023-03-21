// #![feature(const_trait_impl)]
mod mmaurice;
mod config;

use mmaurice::{
    NodeDataDisplayGuarded,
};

#[macro_use]
mod utils;
use std::sync::{Arc, Mutex};
use std::thread;

use actix_web::{get, App, HttpResponse, HttpRequest, HttpServer, Responder, web::{self, Data}};

#[get("/test")]
async fn test2(data: Data<NodeDataDisplayGuarded>) -> impl Responder{
    let data = data.lock().unwrap();
    println!("data: {:?}", data.nodes[0].coords.x);
    // let json_str = json_to_string(&data).unwrap();
    //println!("sending x:{}",data.nodes[0].coords.x);
    HttpResponse::Ok().json(&*data)
    // json_str // <- send json response
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    // let my_struct = DataProvidedToFrontend {
    //     positions_json: "{counter: -1}".to_string(),
    // };
    // let my_struct = Arc::new(Mutex::new(my_struct));

    // let my_struct_clone = my_struct.clone();
    // thread::spawn(move || {
    //     let mut counter:u32 = 0;
    //     loop {
    //         {
    //             let mut my_struct = my_struct_clone.lock().unwrap();
    //             my_struct.positions_json = format!("{{\"counter\": {}}}", counter);
    //             counter += 1;
    //         }
    //         // println!("counter: {}", counter);
    //         thread::sleep(Duration::from_secs(1));
    //     }
    // });
    let config = config::Config::new();
    let mut maurice = mmaurice::Maurice::new(config);

    let node_data_display = maurice.get_node_data_display();

    thread::spawn(move || {
        maurice.run();
    });

    let frontend_location = String::from("../frontend/build");
    HttpServer::new(move || {
        App::new()
            .app_data(Data::new(node_data_display.clone()))
            .service(test2)
            .service(actix_files::Files::new("/", frontend_location.clone())
                .index_file("index.html")
                .default_handler(
                    actix_files::NamedFile::open(
                        vec![frontend_location.clone(), "index.html".to_string()].join("/"),
                    ).expect("failed to open index.html"),
                ),
            )
        })
    .bind("127.0.0.1:6699")?
    .run()
    .await
}