use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::thread;
use std::time::Duration;

fn main() {
    let mut latest_file = find_latest_file().expect("Failed to find latest file");
    let mut last_line = String::new();
    let mut repeat_count = 0;
    loop {
        // Open the file for reading
        let file = File::open(&latest_file).expect("Failed to open file");

        // Create a BufReader to efficiently read from the file
        let mut reader = BufReader::new(file);

        // Read any existing lines in the file
        let mut line = String::new();
        while reader.read_line(&mut line).unwrap() > 0 {
            print!("{}", line);
            line.clear();
        }

        // Poll for new lines in the file
        loop {
            thread::sleep(Duration::from_millis(100));

            let mut line = String::new();
            if reader.read_line(&mut line).unwrap() > 0 {
                print!("{}", line);
                continue;
            }

            // Check for a new file
            match find_latest_file() {
                Ok(path) if path != latest_file => {
                    latest_file = path;
                    break;
                }
                Ok(_) => {}
                Err(_) => {
                    // TODO: handle error
                }
            }
        }
    }
}

fn find_latest_file() -> Result<String, std::io::Error> {
    // Get the current working directory
    let current_dir = env::current_dir()?;

    let mac = "data_stream/04e9e5140f14/cli_data";
    let current_dir = format!("{}/{}", current_dir.to_str().unwrap(), mac);

    // Get a list of all files in the current directory
    let mut files = std::fs::read_dir(current_dir)?
        .map(|res| res.map(|entry| entry.path()))
        .collect::<Result<Vec<_>, std::io::Error>>()?;

    // Sort the files by modification time in reverse order
    files.sort_by_key(|path| path.metadata().unwrap().modified().unwrap());
    files.reverse();

    // Find the first file with an ISO 8601 conforming filename
    for file in files {
        if let Some(filename) = file.file_name().and_then(|n| n.to_str()) {
            if is_iso8601_filename(filename) {
                return Ok(file.to_str().unwrap().to_string());
            }
        }
    }

    Err(std::io::Error::new(
        std::io::ErrorKind::NotFound,
        "No ISO 8601 conforming files found in current directory",
    ))
}

fn is_iso8601_filename(filename: &str) -> bool {
    if filename.len() != 19 {
        return false;
    }
    let parts: Vec<&str> = filename.split("_").collect();
    if parts.len() != 2 {
        return false;
    }
    let date_part = parts[0];
    let time_part = parts[1];
    let date_parts: Vec<&str> = date_part.split("-").collect();
    let time_parts: Vec<&str> = time_part.split("-").collect();
    if date_parts.len() != 3 || time_parts.len() != 3 {
        return false;
    }
    for part in date_parts.iter().chain(time_parts.iter()) {
        if !part.chars().all(|c| c.is_ascii_digit()) {
            return false;
        }
    }
    true
}
