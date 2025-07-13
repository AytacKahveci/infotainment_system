## 🚗 Automotive Infotainment System
This project demonstrates a modular automotive infotainment system, designed to showcase microservice architecture, gRPC-based inter-service communication, and a responsive Qt User Interface (UI).

## ✨ Features
Modular Design: Each core infotainment function operates as an independent service.

gRPC Communication: Services communicate seamlessly using high-performance gRPC, demonstrating both unary (request/response) and streaming RPC patterns.

Qt User Interface: A modern and intuitive UI built with Qt provides the main interaction point for users.

## 🚀 Architecture Overview
The system is composed of several independent services, each handling a specific domain of infotainment functionality.

### 🎵 Media Service
Responsible for all audio and media playback operations. This service handles:

- Playing, pausing, and stopping audio tracks.

- Managing playback status (current track, artist, elapsed time).

- Volume control.

- Streaming real-time playback status updates to the UI.

### 🗺️ Navigation Service
Manages map data and navigation functionalities. This service simulates:

- Handling destination input.

- Providing route guidance.

- Processing location updates (simulated GPS data).

- Streaming navigation instructions and map updates to the UI.

### 🖥️ Qt Infotainment UI
The central user interface for the entire infotainment system. It acts as a gRPC client, interacting with all other services to:

- Display media playback information and controls.

- Render navigation maps and instructions.

- Process user input and send commands to the respective services.

## 🛠️ Installation
Dependencies for this project are efficiently managed using Conan, the C/C++ package manager.

```bash
mkdir build
cd build
conan install .. --build=missing
```