use std::io::Write;
use std::io::stdin;
use std::sync::mpsc::{channel, Sender};
use std::thread;
// use std::time::Duration;
// use std::time::Instant;
// use std::io::BufWriter;
// use std::fs::File;
// use std::io::Write;
use std::fs::OpenOptions;
use std::io::prelude::*;
use std::sync::Arc;

use super::AdcMsgToWrite;

use super::{
    Command, CommandServer, AdcRecMetadata, SoundEffect, ClientSocketWrapper};
use crate::{do_nothing, config::Config, config::MSG_SIZE_BYTES};


// pub struct Msg{
//     pub mac: [u8; 6],
//     pub bytes: [u8; MSG_SIZE_BYTES],
// }

// pub struct MsgToWrite{
//     pub file_path: String,
//     pub msg: Msg,
// }
// pub fn start_writer_thread() -> Sender<MsgToWrite> {
//     let (tx, rx) = channel::<MsgToWrite>();
//     thread::spawn(move || loop {
//         if let Ok(msg) = rx.try_recv() {
//             let file_path = msg.file_path;
//             // ...
//         }
//     });
//     tx
// }
pub fn start_writer_thread() -> Sender<AdcMsgToWrite> {
    let (tx, rx) = channel::<AdcMsgToWrite>();

    thread::spawn(move || {
        loop {
            if let Ok(msg) = rx.try_recv() {
                if let Err(e) = write_msg_to_file(&msg) {
                    eprintln!("Error writing message to file: {}", e);
                }
            }
        }
    });

    tx
}

fn write_msg_to_file(msg_to_write: &AdcMsgToWrite) -> std::io::Result<()> {
    let file_path = &msg_to_write.file_path;
    let msg = &msg_to_write.msg;

    let file = OpenOptions::new()
        .append(true)
        .create(true)
        .open(file_path)?;

    let mut buf_writer = std::io::BufWriter::new(file);

    // Write the MAC address as a string
    // let mac_str = format!("{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}", 
    //                        msg.mac[0], msg.mac[1], msg.mac[2], msg.mac[3], msg.mac[4], msg.mac[5]);
    // write!(buf_writer, "{} ", mac_str)?;

    // Write the message bytes as a hex string
    for byte in msg.bytes.iter() {
        write!(buf_writer, "{:02X}", byte)?;
    }
    writeln!(buf_writer)?;

    Ok(())
}
