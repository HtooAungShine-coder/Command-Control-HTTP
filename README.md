# Command-Control-HTTP
This is a command and control server and agent which i build only for lab testing and code inspiration purpose. This is solely based on HTTP so this does not encrypt commands or authenticate the agent. So you cannot really use this code in reality. This is not a Malware , yet.


# C++ HTTP Command & Control (C2) Framework

A lightweight, multi-threaded Command and Control (C2) proof-of-concept written in C++. The project consists of a stealthy Windows endpoint agent and a dual-interface server featuring a public-facing API for agent communications alongside an isolated local administrative web dashboard.

## Architecture Overview

The framework is split into two primary components that communicate asynchronously over HTTP using the cpp-httplib library:

1. The Server (C2 Controller)
* Local Dashboard Server (127.0.0.1:8080/panel): A private UI that reads a Dashboard.html file, dynamically injects active agent logs and pending command queues via string placeholders, and handles task submissions.
* Public Agent Server (0.0.0.0:6060): A public-facing API. Agents fetch queued commands via GET requests to /getsnack and exfiltrate terminal outputs via POST requests to /givesnack.

2. The Agent (Implant)
* A stealthy Windows binary that automatically hides its console window on startup using native Windows API calls. It continuously beacons the public server every 10 seconds, fetches pending commands, executes them using native pipes, handles stateful directory navigation, and pushes results back to the controller.

## Features

Server Side:
* Dual-Port Isolation: Keeps admin dashboard traffic strictly on localhost:8080 while opening 0.0.0.0:6060 to external agent traffic.
* Multi-Threaded: Uses C++ standard threads to run both server instances concurrently without blocking.
* Dynamic HTML Templating: Replaces placeholder tokens in a static HTML file on-the-fly to show active agents, pending tasks, and console execution history.
* In-Memory Tracking: Utilizes maps, queues, and vectors to map commands and outputs individually by agent IP address.

Agent Side:
* Stealth Initialization: Instantly detaches and suppresses the visible command window using the native Win32 API (ShowWindow).
* Stateful Directory Trajectory: Intercepts 'cd' requests manually and executes '_chdir' to preserve the active working directory across isolated command strings.
* Error Aggregation: Appends '2>&1' to system commands so both standard output and standard error are captured and sent back to the dashboard.
* Connection Resilience: Sets precise 5-second connection and 6-second read timeouts to prevent the binary from hanging permanently during network failures.

## What This Program Does NOT Cover

This project is an educational proof-of-concept demonstrating basic HTTP beaconing mechanisms. To maintain simplicity, the following production-grade and operational security (OPSEC) features are not covered:

1. Traffic Encryption (Cleartext HTTP)
All communication travels over plaintext HTTP. Commands and exfiltrated command data are fully visible to packet sniffers and Network Intrusion Detection Systems (NIDS). No HTTPS/TLS or custom payload encryption/obfuscation is implemented.

2. Agent Authentication & Verification
The server accepts incoming requests from any IP address blindly and maps them solely by their remote address. There is no pre-shared key (PSK), token verification, or agent registration handshake.

3. Data Persistence
The server keeps all agent tracking maps, queues, and terminal histories entirely in-memory. If the server process crashes or restarts, all logs, tracks, and queued commands are permanently lost.

4. Interactive Shell Streams
Commands are handled completely non-interactively using short-lived isolated pipes (_popen). Complex interactive CLI utilities (like launching powershell.exe interactively, SSH, or utilities requiring continuous keyboard feedback/prompts) will hang or fail.

5. Advanced AV/EDR Evasion
Aside from a basic SW_HIDE window call, the agent features no evasion tactics (such as API hashing, run-time dynamic linking, memory-only execution, or code obfuscation) and will likely be flagged quickly by modern Endpoint Detection and Response (EDR) agents.

## Dependencies & Building

Dependencies:
* C++ Compiler: A compiler supporting C++17 or later (e.g., MSVC for Windows Agent, GCC/Clang for the Server).
* Networking Library: yhirose/cpp-httplib (Header-only library, included locally as "httplib.h").

Compilation Notes:
* Agent Bug Fix: In the provided agent code snippet, ensure the line 'while(fgets(space, sizeof(space), pipe) != nullptr)' is updated to use the declared array buffer: 'while(fgets(buffer, sizeof(buffer), pipe) != nullptr)'.

Windows Agent Build (MSVC):
cl.exe /EHsc /std:c++17 agent.cpp /link user32.lib

Server Build (Linux/GCC):
g++ -std=c++17 server.cpp -o c2_server -pthread

## Disclaimer

This project is created exclusively for educational purposes, authorized security auditing, and penetration testing research. Utilizing this software against targets without prior written consent is strictly illegal. The author assumes no liability for misuse, legal ramifications, or damage caused by this program.
