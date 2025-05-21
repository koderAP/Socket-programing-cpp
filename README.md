# Socket Programming Project

This repository contains a modular implementation of client-server communication protocols and scheduling algorithms using TCP sockets and C++. The project is divided into four parts, each exploring different communication patterns, concurrency models, and scheduling policies.

## Repository Structure

```bash
.
├── part1/                     # Word Counting Client
│   ├── client.cpp
│   ├── server.cpp
│   ├── config.json
│   ├── Makefile
│   └── ...
├── part2/                     # Concurrent Clients
│   ├── client.cpp
│   ├── server.cpp
│   ├── config.json
│   ├── Makefile
│   └── ...
├── part3/                     # Grumpy Server (Decentralized Scheduling)
│   ├── client.cpp
│   ├── server.cpp
│   ├── config.json
│   ├── Makefile
│   └── ...
├── part4/                     # Friendly Server (Centralized Scheduling)
│   ├── client.cpp
│   ├── server.cpp
│   ├── config.json
│   ├── Makefile
│   └── ...
├── report.pdf                 # Final Report
└── plot.png                   # Completion Time Plots
```



## Part 1: Word Counting Client

Implements a TCP client-server system where the client reads a list of words from a file hosted on the server and counts the frequency of each word. The server responds with a fixed number (`k`) of words starting from a requested offset, sending `p` words per packet.

###  Files
- `client.cpp`
- `server.cpp`
- `config.json`
- `Makefile`

###  How to Run
```bash
make build       # Builds client and server
make run         # Runs the client and server
make plot        # Outputs a plot of average completion time vs packet size
```

---

## Part 2: Concurrent Clients with Word Counting


Extends the server to handle multiple concurrent TCP clients using multithreading. Measures the impact of concurrency on average completion time per client.

###  Files
- `client.cpp`
- `server.cpp`
- `config.json`
- `Makefile`

###  How to Run
```bash
make build       # Builds the code
make run         # Runs server and multiple clients
make plot        # Outputs a plot of completion time per client vs number of clients
```

---

## Part 3: Grumpy Server with Decentralized Scheduling


Implements a decentralized communication model with a server that prefers to serve one request at a time. Clients handle contention using the following protocols:
- Slotted ALOHA
- Binary Exponential Backoff (BEB)
- Sensing with BEB

Clients receive a `HUH!` response if requests collide.

###  Files
- `client.cpp`
- `server.cpp`
- `config.json`
- `Makefile`

###  How to Run
```bash
make build         # Builds the code
make run-aloha     # Runs Slotted ALOHA protocol
make run-beb       # Runs Binary Exponential Backoff protocol
make run-cscd      # Runs Sensing with BEB protocol
make run           # Runs all protocols and logs results
make plot          # Outputs a plot comparing all three protocols
```

---

## Part 4: Friendly Server with Centralized Scheduling


Implements centralized server scheduling using:
- FIFO (First In First Out)
- Fair Scheduling (Round Robin)

Also simulates rogue clients and evaluates fairness using Jain’s index.

###  Files
- `client.cpp`
- `server.cpp`
- `config.json`
- `Makefile`

###  How to Run
```bash
make build         # Builds the code
make run-fifo      # Runs FIFO scheduling
make run-rr        # Runs Round Robin scheduling
make run           # Runs both and logs results
make plot          # Outputs a plot comparing FIFO and Round Robin
make fairness      # Runs fairness experiment and reports Jain’s index
```

---

## 📊 Report

See `report.pdf` for detailed analysis, plots, and observations from all four parts of the project.

---
