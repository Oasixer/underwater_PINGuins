use std::process::Command;
use std::collections::HashSet;

fn main() {
    let program = "usbipd";
    let args = &["wsl", "list"];

    // Save the initial list of connected devices
    let mut connected_devices = HashSet::new();
    let output = Command::new(program)
                     .args(args)
                     .output()
                     .expect("failed to execute process");

    let stdout_str = String::from_utf8_lossy(&output.stdout);
    for line in stdout_str.lines() {
        let columns: Vec<&str> = line.split("  ").collect::<Vec<&str>>().iter().filter(|x| !x.is_empty()).map(|x| x.trim()).collect();

        if columns.len() >= 4 && columns[2].contains("USB Input Device") {
            let busid = columns[0];
            connected_devices.insert(busid.to_owned());
        }
    }

    loop {
        // Run the executable with arguments and capture its output
        let output = Command::new(program)
                         .args(args)
                         .output()
                         .expect("failed to execute process");

        // Parse the output to find the busid of any USB Input Device that is not attached and was not present in the saved list
        let stdout_str = String::from_utf8_lossy(&output.stdout);
        for line in stdout_str.lines() {
            let columns: Vec<&str> = line.split("  ").collect::<Vec<&str>>().iter().filter(|x| !x.is_empty()).map(|x| x.trim()).collect();

            // print columns
            println!("{:?}", columns);
            if columns.len() < 4 {
                continue;
            }
            if (columns.len() >= 4 && columns[2].contains("USB Input Device") && columns[3] == "Not attached") ||
               (columns[2].contains("USB Serial Device") && columns[3] == "Not attached"){
                let busid = columns[0];
                if !connected_devices.contains(busid) {
                    println!("Found unattached USB Input Device with busid {}", busid);

                    // Attach the device using the same program with new args
                    let attach_args = &["wsl", "attach", "--busid", busid];
                    let attach_output = Command::new(program)
                                            .args(attach_args)
                                            .output()
                                            .expect("failed to execute process");

                    // Print the output of the attach command
                    println!("{}", String::from_utf8_lossy(&attach_output.stdout));

                    // Add the device to the list of connected devices
                    // connected_devices.insert(busid.to_owned());
                }
            }
        }

        // Wait for a few seconds before polling again
        std::thread::sleep(std::time::Duration::from_secs(1));
    }
}
