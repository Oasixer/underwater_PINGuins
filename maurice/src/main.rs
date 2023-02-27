mod mmaurice;
mod config;

#[macro_use]
mod utils;

fn main() {
    println!("Hello, world!");

    let config = config::Config::new();
    let mut maurice = mmaurice::Maurice::new(config);
    maurice.run();
} 