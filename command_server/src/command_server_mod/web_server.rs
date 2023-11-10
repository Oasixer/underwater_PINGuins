#![feature(decl_macro)]

use std::time::Instant;
use std::sync::{Arc, Mutex};
use rocket::{get, routes};
use rocket::{Request};
use rocket::request::{FromRequest, Outcome};
use std::thread;
use std::time::Duration;

struct MyStruct {
    my_variable: String,
}
struct MyStructGuard(Arc<Mutex<MyStruct>>);

impl<'a, 'r> FromRequest<'a, 'r> for MyStructGuard {
    type Error = ();

    fn from_request(request: &'a Request<'r>) -> Outcome<Self, Self::Error> {
        match request.guard::<rocket::State<Arc<Mutex<MyStruct>>>>() {
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
    println!("Hello, world!");
    let my_struct = MyStruct {
        my_variable: "Hello, world!".to_string(),
    };

    let my_struct = Arc::new(Mutex::new(my_struct));

    let my_struct_clone = my_struct.clone();
    thread::spawn(move || {
        loop {
            {
                let mut my_struct = my_struct_clone.lock().unwrap();
                my_struct.my_variable = "Hello, universe!".to_string();
            }
            thread::sleep(Duration::from_secs(1));
        }
    });
    
    thread::spawn(move || {
        rocket::ignite()
            .manage(my_struct)
            .mount("/", routes![index])
            .launch();
    });

    loop {
        println!("Hello from main thread");
        thread::sleep(Duration::from_secs(1));
    }

}