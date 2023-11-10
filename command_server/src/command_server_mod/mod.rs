use std::net::{TcpListener, TcpStream};
use std::sync::mpsc;
use std::io::{BufWriter};
use std::sync::mpsc::{channel, Sender, Receiver};
use std::io::Write;
use std::fs::File;
use tokio::task;
use std::sync::{Mutex, Arc};
use std::time::{SystemTime, UNIX_EPOCH};


// use rocket::serde::Serialize;
use serde::Serialize;
use std::time::Instant;


mod server;
mod sound_player_mod;
mod stdin_listener;
mod file_writer;
mod consume_data;
// mod web_server;

use sound_player_mod::SoundPlayer;
use sound_player_mod::SoundEffect;
use crate::config::{
    Config,
    OUTGOING_CMD_SIZE_BYTES,
    MSG_SIZE_BYTES,
    Coord3D,
    Node,
    config_const_nodes,
};
use crate::utils::{create_parent_directories, thread_sleep};

pub struct ConnectionChange{
    pub mac: [u8; 6],
    pub is_connected: bool,
    // pub adc_rec_metadata_changed: Option<AdcRecMetadata>,
}

pub struct ClientSocketWrapper{
    mac: [u8; 6],
    latest_connection_change: Option<ConnectionChange>,
    stream: TcpStream,
    stream_file_name: String,
    stream_file: Option<std::fs::File>,
    adc_rec_metadata: AdcRecMetadata,
    last_hb_received: Instant,
    kill_me: Sender<bool>,
    // tx_cmd_from_stdin_listener_to_client: Sender<Command>,
}

pub struct Msg{
    pub mac: [u8; 6],
    pub bytes: [u8; MSG_SIZE_BYTES],
}

pub struct AdcMsgToWrite{
    pub file_path: String,
    pub msg: Msg,
}

// #[derive(std::convert::AsRef)]
pub struct CommandServer{
    sound_player: SoundPlayer,
    config: Config,
    // client_sockets: Vec<TcpStream>,
    client_sockets: Vec<ClientSocketWrapper>,
    server: TcpListener,
    tx_msg_from_client_listener_to_consumer: Sender<Msg>,
    rx_msg_consumer: Receiver<Msg>,
    tx_connection_change: Sender<ConnectionChange>,
    rx_connection_change: Receiver<ConnectionChange>,
    rx_stdin_listener: Receiver<String>,
    tx_adc_file_writer: Sender<AdcMsgToWrite>,
    data_provided_to_frontend: NodeDataDisplayGuarded,
    // tx_display_data: Sender<DisplayData>,
    // rx_file_writer: Receiver<MsgToWrite>,
}

// pub struct Message<'r> {
//    contents: &'r str,
// }

// #[derive(Serialize)]
// pub struct DisplayData<'r>{
//     coords: &'r Coord3D,
//     updated: &'r std::time::Instant,
// }

// struct DataProvidedToFrontend {
//     positions_json: String,
// }

#[derive(Serialize)]
pub struct NodeDataDisplay{
    // coords: Coord3D,
    pub nodes: [Node; 4],
    updated: u128,
}
pub type NodeDataDisplayGuarded = Arc<Mutex<NodeDataDisplay>>;
fn to_guarded(NodeDataDisplay: NodeDataDisplay) -> NodeDataDisplayGuarded{
    Arc::new(Mutex::new(NodeDataDisplay))
}



#[derive(Clone)]
pub struct AdcRecMetadata{
    socket_mac: [u8; 6],
    tx_adc_file_writer: Sender<AdcMsgToWrite>,
    file_prefix: Option<String>,
    file_suffix: Option<i32>,
    file_name: Option<String>,
    duration: Option<std::time::Duration>,
    end_time: Option<std::time::Instant>,
    // is_continuation: bool,
}

impl AdcRecMetadata{
    pub fn write(&self, bytes: [u8; MSG_SIZE_BYTES]){//, tx_file_writer: Sender<MsgToWrite>) {
        // let tx_file_writer = tx_file_writer.clone();
        if let Some(file_name) = &self.file_name {
            let msg = Msg{
                mac: self.socket_mac,
                bytes,
            };
            let msg_to_write = AdcMsgToWrite{
                file_path: file_name.clone(),
                msg,
            };
            if let Err(e) = self.tx_adc_file_writer.send(msg_to_write) {
                eprintln!("Error sending message to file writer: {}", e);
            }
        }
    }
}


// impl AdcRecMetadata{
//     pub fn write_stream() 
// }

struct Command{
    target_mac: [u8; 6],
    bytes: [u8; OUTGOING_CMD_SIZE_BYTES],
}


impl CommandServer {

    pub fn get_node_data_display(&self) -> NodeDataDisplayGuarded{
        self.data_provided_to_frontend.clone()
    }

    // hashmap mapping last 2 digits of mac address to full address
    pub fn replace_client_socket(&mut self, mac: [u8; 6], new_client_socket: ClientSocketWrapper) {
        let mut index = 0;
        for client_socket in &self.client_sockets {
            if client_socket.mac == mac {
                self.client_sockets[index] = new_client_socket;
                break;
            }
            index += 1;
        }
    }
    pub fn get_client_socket(&mut self, mac: [u8; 6]) -> Result<&mut ClientSocketWrapper, String> {
        self.client_sockets.iter_mut().find(|client_socket| client_socket.mac == mac).ok_or_else(|| format!("Could not find client socket with mac: {:?}", mac))
    }
    // pub fn get_client_socket_immut(t self, mac: [u8; 6]) -> Result<&mut ClientSocketWrapper, String> {
    //     self.client_sockets.iter_mut().find(|client_socket| client_socket.mac == mac).ok_or_else(|| format!("Could not find client socket with mac: {:?}", mac))
    // }

    pub fn run(&mut self) {
        // let rocket_handle = task::spawn(async move {
        //         rocket().launch().await;
        //     });
        loop {
            self.accept_new_connections_nonblocking();
            self.poll_for_messages_passed_from_socket_polling_threads();
            self.poll_for_lines_from_stdin_listener();
            self.poll_for_connection_changes();
            self.poll_for_finished_recording();
            // self.client_publisher();
        }
        // rocket_handle.await.unwrap();

    }

    pub fn new(config: Config) -> Self {
        let sound_player = SoundPlayer::new();

        let client_sockets: Vec<ClientSocketWrapper> = vec![];
        // let config = Arc::new(config);
        let config = config.clone();
        let server_result = TcpListener::bind(format!("{}:{}",config.my_ip.as_str(), config.server_port));
        let server: TcpListener = match server_result {
            Ok(server) => server,
            Err(e) => {
                sound_player.play_sound_effect(sound_player_mod::SoundEffect::Error);
                eprintln!("Tried to bind this ip: {}, received error: {}",config.my_ip.as_str(), e);
                println!("!!! YOU ARE ON THE WRONG NETWORK !!!");
                thread_sleep(500);
                std::process::exit(0);
            }
        };
        server
            .set_nonblocking(true)
            .expect("failed to initialize non-blocking");

        println!("Waiting for connections...");


        let (tx_msg_from_client_listener_to_consumer, 
             rx_msg_consumer) = channel::<Msg>();
        // let (tx_cmd_from_stdin_listener_to_client,
        //      rx_client_publisher) = mpsc::channel::<Command>();
        
        let (tx_connection_change, rx_connection_change) = mpsc::channel::<ConnectionChange>();
        let rx_stdin_listener = stdin_listener::start_stdin_listener_thread2();

        let tx_file_writer = file_writer::start_writer_thread();

        let data_provided_to_frontend = to_guarded(NodeDataDisplay{
            nodes: config_const_nodes(),
            updated: 0,
        });

        // let tx_display_data = web_server::start_web_server_thread();

        CommandServer {
            sound_player,
            config,
            client_sockets,
            server,
            tx_msg_from_client_listener_to_consumer,
            rx_msg_consumer,
            // tx_cmd_from_stdin_listener_to_client,
            // rx_client_publisher,
            tx_connection_change,
            rx_connection_change,
            rx_stdin_listener,
            tx_adc_file_writer: tx_file_writer,
            data_provided_to_frontend,
            // tx_display_data
            // client_socket_adc_rec_metadatas
        }
    }
}
