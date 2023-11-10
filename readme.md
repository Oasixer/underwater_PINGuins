# Pinguins - Distributed Underwater Positioning System

<p align="center">Software for Pinguins, 2023 Winner of 'Best Overall Project' at U of Waterloo Mechatronics Eng. Capstone Design Symposium. The project used 4 custom-made underwater acoustic beacons with one mounted to a moving underwater ROV and was able to locate the ROV (like underwater GPS). We achieved 85+ meter range, and ~98% accuracy even at smaller scales. This repo contains [firmware for underwater beacons](#firmware), the [command and control server (rust)](#command server), and the [dashboard UI](#dashboard).
</p>

![image](https://kaelan.xyz/images/portfolio/pinguins/underwater/full.jpg)

## Pinguins table of contents

<!--toc:start-->

- [System Overview](#system-overview)
  - [Firmware Overview](#firmware-overview)
  - [Command Server Overview](#command-server-overview)
  - [Dashboard Overview](#dashboard-overview)
- [Firmware](#firmware)
  - [Requirements](#firmware-requirements)
  - [Installation](#firmware-installation)
  - [Usage](#firmware-usage)
- [Command Server](#command-server)
  - [Requirements](#command-server-requirements)
  - [Installation](#command-server-installation)
  - [Usage](#command-server-usage)
  - [CLI](#commmand-server-cli)
- [Dashboard](#dashboard)
  - [Requirements](#dashboard-requirements)
  - [Installation](#dashboard-installation)
  - [Usage](#dashboard-usage)

<!--toc:end-->

# System overview
![image](https://github.com/Oasixer/underwater_PINGuins/assets/24990515/ebc98be9-f576-4526-b10f-b761287a96c9)

- Each Node, including the ROV, communicates with the orchestrating [Command Server](#command-server).
- The Rust Server accepts TCP connections from Nodes and accepts web requests for updated data from the dashboard.
- The nodes send debug information over TCP such as on sending or receiving an acoustic signal or the detected frequency magnitudes.
- Additionally, the ROV node calculates its position and sends it to the server as a different message type over the same TCP connection.

## Firmware Overview

- All key algorithms for the system are implemented in the Node firmware, as the data processing is performed onboard as is the position calculation.
- The firmware, implemented in CPP, has two configurations: Stationary Node, and ROV Node.
  - Each Node has a default configuration, but can be set to operate as ROV or Stationary Node 1-3 during runtime if needed, via a command from the server.
- The Stationary Node firmware revolves around a state-machine in which each node waits to hear its specific frequency, which triggers it to reply, then return to listening again.
- To facilitate this, an ISR is triggered at 500KHz in the background, which appends to a ring buffer and updates the detected frequency magnitudes by incrementing the sliding window Fourier transform with the ring buffer as the input data.
- The ROV configuration shares most of its code with the Stationary configuration but wraps it with a different state machine.
  - The ROV configuration has two modes of its own: Calibration and Continuous.
  - In Calibration mode, the ROV exchanges acoustic pulses with the stationary nodes in groups of two, measuring the time difference in responses between each node and its sibling.
  - Many cycles of this are performed, to get an accurate distance from the ROV to each Node and each Node to its sibling.
  - These distances are sufficient to resolve a 3D space, provided the depth of each Node is known.
- In Continuous mode, the ROV interacts with the stationary nodes one at a time, multilaterating its own position with the distances calculated each cycle and sending the updated position to the server.
- Continuous mode depends on having already resolved a 3D space with known positions for each of the stationary nodes.
- The Node positions can be resolved by running calibration or setting them via a global command from the Server CLI.

## Command Server Overview

- The Rust server accepts TCP connections from each of the Stationary nodes, differentiating them using their MAC addresses as IDs.
- It can [receive commands over CLI during system setup/calibration](#command-server-cli), which it forwards to one or all nodes, depending on the command.
- As part of the debug information, each node informs the server when it receives or sends a signal acoustically, to diagnose any occurrence of missed or misheard signals during operation.
- The server receives position updates from the ROV node and it serves these to the dashboard on demand to provide the visualization.
- Other supporting data that is sent to the dashboard includes connection status, and timestamps of last acoustic signals, both transmitted and received.

## Dashboard Overview

- The dashboard is written in SvelteJS and displays a live map showing live position updates
- It is served by the [Command Server](#command-server) which uses [Rocket](https://rocket.rs) as a web server in the background

- image go here!!!!!!!!!!!!!!!!!

# Firmware

## Firmware Requirements

- The firmware is designed for the Teensy 4.1 (ARM Cortex-M7) which runs on our custom PCBs in each beacon.
- Windows, Linux, and Mac are supported for building & uploading the firmware

## Firmware Installation

- Can be built using a custom build script and docker and no Arduino but this is WIP
- for now just use Arduino IDE w/ the Teensy plugin and compile + upload `firmware/main/main.ino`.

## Firmware Usage

- Each Node is designed to be set-and-forget (floating in the water, wireless), and with [auto calibration](#auto-calibration), even the location of each floating node need not be known ahead of time.
- However, accuracy may be higher if the positions of each floating node are measured and hardcoded at compile for a particular operation.
- See [configuration](#firmware-configuration) for details, but note that all configuration parameters can be updated at runtime through the [Command Server CLI](#command-server-cli)

# Command Server (Rust)

## Command Server Requirements

- rust version >= 1.7.3
  - Install with:

```console
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

## Command Server Installation

## Command Server CLI

```console
$ cargo build
```

Cargo install
(nightly? etc? get history from yog)
