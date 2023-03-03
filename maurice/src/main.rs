mod mmaurice;
mod config;

#[macro_use]
mod utils;

#[macro_use] extern crate rocket;

fn main() {
    let config = config::Config::new();
    let mut maurice = mmaurice::Maurice::new(config);
    maurice.run();
} 