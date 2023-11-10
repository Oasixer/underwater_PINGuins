use rodio::{Decoder, OutputStream, Sink};
use std::io::{BufReader, Read, Write, Cursor};
use strum::IntoEnumIterator;
use strum_macros::{EnumIter, Display};
use std::collections::HashMap;
use std::sync::mpsc::{channel, Receiver, Sender, TryRecvError};
use std::fs::File;
use std::thread;
use crate::utils::thread_sleep;

const VOLUME_INCREMENT : f32 = 0.1;
const VOLUME_MAX: f32 = 1.0;
const VOLUME_MIN: f32 = 0.0;
const VOLUME_DEFAULT: f32 = 0.3;

#[derive(EnumIter, Debug, PartialEq, Eq, Display, Hash)]
pub enum SoundEffect {
    Connect,
    Disconnect,
    Overrun,
    Error,
    Error2,
    Leak,
    Leak2,
    StartRecording,
    EndRecording,
    ReduceVolume,
    IncreaseVolume,
    StillAlive0,
    StillAlive1,
}

impl SoundEffect {
    fn filename(&self) -> String {
        format!("sounds/{}.mp3", self.to_string().to_lowercase())
    }
}

pub struct SoundPlayer {
    // sounds_in_memory: HashMap<SoundEffect, Vec<u8>>,
    // sink: Sink,
    pub audioplayer_tx: Sender<SoundEffect>,
    // audioplayer_rx: Receiver<SoundEffect>,
}

impl SoundPlayer {
    pub fn new() -> Self {
        let (audioplayer_tx, audioplayer_rx): (Sender<SoundEffect>, Receiver<SoundEffect>) =
            channel();


        // let (_stream, stream_handle) = OutputStream::try_default().unwrap();
        // let sink = Sink::try_new(&stream_handle).unwrap();
        // sink.set_volume(VOLUME_DEFAULT);
        // let sound_effect = SoundEffect::StillAlive0;
        // let sound_raw = sounds_in_memory[&sound_effect].clone();
        // let cursor = Cursor::new(sound_raw);
        // let source: Decoder<Cursor<Vec<u8>>> = rodio::Decoder::new(cursor).unwrap();

        // let mut vol = VOLUME_DEFAULT;
        // sink.append(source);
        // sink.play();
        // sink.sleep_until_end();

        thread::spawn(move || {
            // let audioplayer_rx = audioplayer_rx.clone();
            let sounds_in_memory: HashMap<SoundEffect, Vec<u8>> = SoundEffect::iter()
            .map(|s| {
                let file = File::open(s.filename()).unwrap();
                let mut reader: BufReader<File> = BufReader::new(file);
                let mut buffer: Vec<u8> = Vec::new();
                reader.read_to_end(&mut buffer).unwrap();
                (s, buffer)
            })
            .collect();

            let (_stream, stream_handle) = OutputStream::try_default().unwrap();
            let sink = Sink::try_new(&stream_handle).unwrap();
            sink.set_volume(VOLUME_DEFAULT);

            loop {
                match audioplayer_rx.recv() {
                    Ok(sound_effect) => {

                        // let (_stream, stream_handle) = OutputStream::try_default().unwrap();
                        // let sink = Sink::try_new(&stream_handle).unwrap();
                        // // sink.set_volume(VOLUME_DEFAULT);
                        // let sound_effect = sound_effect;
                        let sound_raw = sounds_in_memory[&sound_effect].clone();
                        let cursor = Cursor::new(sound_raw);
                        let source: Decoder<Cursor<Vec<u8>>> = rodio::Decoder::new(cursor).unwrap();
                        sink.append(source);
                        sink.play();
                        sink.sleep_until_end();
                        // let sound_raw = sounds_in_memory[&sound_effect].clone();
                        // let cursor = Cursor::new(sound_raw);
                        // let source: Decoder<Cursor<Vec<u8>>> = rodio::Decoder::new(cursor).unwrap();
                        // sink.append(source);
                        // println!("Playing sound");
                        // sink.play();
                        // sink.sleep_until_end();

                        if sound_effect == SoundEffect::IncreaseVolume {
                            let new_volume = sink.volume() + VOLUME_INCREMENT;
                            if new_volume <= VOLUME_MAX {
                                sink.set_volume(new_volume);
                            }
                        } else if sound_effect == SoundEffect::ReduceVolume {
                            let new_volume = sink.volume() - VOLUME_INCREMENT;
                            if new_volume >= VOLUME_MIN {
                                sink.set_volume(new_volume);
                            }
                        }
                    }
                    Err(_) => println!("Error receiving sound command"),
                }
                thread_sleep(100);
            }
        });

        SoundPlayer {
            // sounds_in_memory,
            // sink,
            audioplayer_tx,
            // audioplayer_rx,
        }
    }

    pub fn play_sound_effect(&self, sound_effect: SoundEffect) {
        self.audioplayer_tx.send(sound_effect).unwrap();
    }
}

impl AsRef<SoundPlayer> for SoundPlayer {
    fn as_ref(&self) -> &Self {  // Self is Struct<'a>, the type for which we impl AsRef
        self
    }
}