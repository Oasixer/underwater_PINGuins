use std::io;
use std::fs;
use std::path::Path;

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
        (None, None)
    };
}

pub fn create_directories(path: &str) -> std::io::Result<()> {
    let dir_path = Path::new(path).parent().unwrap();
    fs::create_dir_all(dir_path)
}
