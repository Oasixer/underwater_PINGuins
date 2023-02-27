mod mmaurice;
mod config;

#[macro_use]
mod utils;

fn main() {
    let config = config::Config::new();
    let mut maurice = mmaurice::Maurice::new(config);
    maurice.run();
} 