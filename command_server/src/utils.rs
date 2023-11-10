use std::io;
use std::fs;
use std::path::Path;
use std::thread;
use std::time::{SystemTime, UNIX_EPOCH};

#[macro_export]
macro_rules! impl_display_for_enum {
    ($enum_type:ty) => {
        impl std::fmt::Display for $enum_type {
            fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
                write!(f, "{:?}", self)
            }
        }
    };
}

#[macro_export]
macro_rules! iterable_enum {
    ($visibility:vis, $name:ident, $($member:tt),*) => {
        $visibility enum $name {$($member),*}
        impl $name {
            fn iterate_me() -> Vec<$name> {
                vec![$($name::$member,)*]
            }
        }
    };
    ($name:ident, $($member:tt),*) => {
        iterable_enum!(, $name, $($member),*)
    };
}

#[macro_export]
macro_rules! hashmap {
    ($( $key: expr => $val: expr ),*) => {{
         let mut map = ::std::collections::HashMap::new();
         $( map.insert($key, $val); )*
         map
    }}
}

#[macro_export]
macro_rules! do_nothing {
    () => {
        None
    };
}

pub fn create_parent_directories(path: &str) -> std::io::Result<()> {
    let dir_path = Path::new(path).parent().unwrap();
    fs::create_dir_all(dir_path)
}
pub fn create_directories(path: &str) -> std::io::Result<()> {
    let dir_path = Path::new(path);
    fs::create_dir_all(dir_path)
}

pub fn hex_string_to_u8(hex_str: &str) -> Result<u8, String> {
    u8::from_str_radix(hex_str, 16).map_err(|_| format!("Invalid hexadecimal string: {}", hex_str))
}

pub fn thread_sleep(millis: u64) {
    thread::sleep(std::time::Duration::from_millis(millis));
}


pub fn get_posix_millis() -> u128 {
    let start = SystemTime::now();
    let since_the_epoch = start
        .duration_since(UNIX_EPOCH)
        .expect("Time went backwards");
    let in_ms = since_the_epoch.as_millis();
    in_ms
}