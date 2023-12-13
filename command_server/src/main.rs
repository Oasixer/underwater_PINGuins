// #![feature(const_trait_impl)]
mod command_server_mod;
mod config;

use command_server_mod::{
    NodeDataDisplayGuarded,
};

#[macro_use]
mod utils;
use std::sync::{Arc, Mutex};
use std::thread;

use actix_web::{get, App, HttpResponse, HttpRequest, HttpServer, Responder, web::{self, Data}};

#[get("/getNodeData")]
async fn getNodeData(data: Data<NodeDataDisplayGuarded>) -> impl Responder{
    let data = data.lock().unwrap();
    // println!("data: {:?}", data.nodes[0].coords.x);
    // let json_str = json_to_string(&data).unwrap();
    //println!("sending x:{}",data.nodes[0].coords.x);
    HttpResponse::Ok().json(&*data)
    // json_str // <- send json response
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    let config = config::Config::new();
    let mut command_server = command_server_mod::CommandServer::new(config);

    let node_data_display = command_server.get_node_data_display();

    thread::spawn(move || {
        command_server.run();
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
