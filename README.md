![AthosCat Logo](./img/athoscat.png)

## Overview

AthosCat is a lightweight network tool inspired by Netcat, designed for establishing TCP and UDP connections using IPv4 or IPv6. It supports two main modes:

- **Client mode**: Connects to remote servers to send and receive data.
- **Listener mode**: Opens a local port to accept incoming connections or datagrams.

## Current State

- **Client Mode**:  
  - Supports TCP and UDP protocols.  
  - Can connect to IPv4 and IPv6 addresses.  
  - Uses `select()` to handle simultaneous input/output from stdin and network socket.  
  - Allows sending data interactively to remote servers.

- **Listener Mode**:  
  - Supports TCP and UDP protocols.  
  - Binds to a local port and listens for incoming connections (TCP) or datagrams (UDP).  
  - For TCP, accepts a single client and prints received data to stdout.  
  - For UDP, receives datagrams with a configurable timeout.

## Usage Example

### Run the listener (server) on port 8080 TCP:

```bash
./athoscat --listener -p 8080 -v
```

### Connect with the client to the listener:

```bash
./athoscat --client -h 127.0.0.1 -p 8080 -v
```

Then type messages to send.

## Future Improvements

- Enhanced error handling and recovery.  
- Support multiple simultaneous clients in listener mode.  
- Add encryption and proxy support.  
- Improve buffering and user interface.

---

Feel free to explore and contribute!
