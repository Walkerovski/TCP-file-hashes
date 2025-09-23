# TCP IP hash server for multiple clients

## Overview

This project involves implementing both a TCP client and server that work together to compute hashes for file segments. The client sends data segments to the server, and the server responds with SHA256 hashes of those segments.

## Protocol

The protocol uses four message types:
1. **Initialization** (Client → Server): Tells server how many hash requests to expect
2. **Acknowledgement** (Server → Client): Confirms receipt and total response length
3. **HashRequest** (Client → Server): Contains data segment to be hashed
4. **HashResponse** (Server → Client): Contains computed hash of the segment

## Server Implementation

### Usage
```bash
server -p <port> [-s <salt>]
```

### Arguments
- `-p <Number>`: Port to bind to and listen on (must be > 1024)
- `-s <String>`: Optional salt for hash computation (ASCII string)

### Example
```bash
server -p 41714 -s newsalt
```

### Requirements
- Uses SHA256 for hashing (OpenSSL wrapper provided)
- Memory limit: 1 MB per client
- Must start sending responses before receiving all requests
- Handles multiple clients concurrently

## Client Implementation

### Usage
```bash
client -a <address> -p <port> -n <count> --smin <min_size> --smax <max_size> -f <file>
```

### Arguments
- `-a <String>`: Server IP address
- `-p <Number>`: Server port number
- `-n <Number>`: Number of hash requests to send (≥ 0)
- `--smin <Number>`: Minimum segment size (≥ 1)
- `--smax <Number>`: Maximum segment size (≤ 2²⁴)
- `-f <File>`: Source file to read data from

### Example
```bash
client -a 128.8.126.63 -p 41714 -n 100 --smin=128 --smax=512 -f /dev/zero
```

### Output Format
Each hash response is printed as:
```
<index>: 0x<hash_in_lowercase_hex>
```

Example:
```
7: 0x147293be17d3bf0e482e44bba5271e3f2cfb1b638b5c59eea2a0fd74c0978509
```