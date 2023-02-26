use rodio::{Decoder, OutputStream, Sink};
use std::io::{BufReader, Read, Write, Cursor};
use strum::IntoEnumIterator;
use strum_macros::{EnumIter, Display};
use std::collections::HashMap;
use std::sync::mpsc::{channel, Receiver, Sender, TryRecvError};
use std::fs::File;
use std::thread;

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
    Leak,
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
    audioplayer_tx: Sender<SoundEffect>,
    // audioplayer_rx: Receiver<SoundEffect>,
}

impl SoundPlayer {
    pub fn new() -> Self {
        let (audioplayer_tx, audioplayer_rx): (Sender<SoundEffect>, Receiver<SoundEffect>) =
            channel();

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
        sink.set_volume(0.3);

        thread::spawn(move || loop {
            // let audioplayer_rx = audioplayer_rx.clone();
            match audioplayer_rx.recv() {
                Ok(sound_effect) => {
                    let sound_raw = sounds_in_memory[&sound_effect].clone();
                    let cursor = Cursor::new(sound_raw);
                    let source: Decoder<Cursor<Vec<u8>>> = rodio::Decoder::new(cursor).unwrap();
                    sink.append(source);
                    sink.play();
                    sink.sleep_until_end();

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