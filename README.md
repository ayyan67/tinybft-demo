# TinyBFT Demo

This repository is a demonstration implementation of the key concepts from TinyBFT (Byzantine Fault-Tolerant Replication for Highly Resource-Constrained Embedded Systems), as described in the IEEE RTAS 2024 paper by Böhm, Distler, and Wägemann.

## Overview

TinyBFT is the first Byzantine fault-tolerant (BFT) state-machine replication library specifically designed for highly resource-constrained embedded devices like ESP32-C3 microcontrollers with just 400KB of RAM. The key innovation is a memory-efficient implementation that reduces memory usage by 98.9% compared to traditional PBFT-based libraries.

This demonstration focuses on:

1. **Static Memory Allocation**: All memory is allocated at compile time, providing guaranteed worst-case memory consumption.

2. **Memory Layout with Four Regions**:
   - **Agreement Region**: Holds protocol certificates for active sequence numbers
   - **Checkpoint Region**: Stores state snapshots and checkpoint certificates
   - **Event Region**: Contains messages with varied lifetimes
   - **Scratch Region**: Provides temporary buffers for message processing

3. **Byzantine Fault Tolerance**: The demo includes simulation of the PBFT protocol flow and demonstrates how the system maintains consistency despite Byzantine (malicious) failures.

## Files in this Repository

- `tinybft_demo.c`: Interactive demonstration of the PBFT protocol with a key-value store
- `memory_layout.h`: Definition of TinyBFT's memory regions and data structures
- `memory_layout.c`: Implementation of the static memory management

## Running the Demo

To run the demo on Windows:

```bash
gcc tinybft_demo.c memory_layout.c -o tinybft_demo.exe
.\tinybft_demo.exe
```

For Unix systems:

```bash
gcc tinybft_demo.c memory_layout.c -o tinybft_demo
./tinybft_demo
```

## Demo Features

The interactive demo allows you to:

1. **PUT**: Add or update key-value pairs, simulating the PBFT protocol flow
2. **GET**: Retrieve values for keys from all replicas
3. **FAULT**: Toggle fault status of replicas to observe Byzantine fault tolerance
4. **PROCESS**: Simulate the PBFT protocol phases with random data
5. **STATUS**: View detailed system status
6. **MEMORY**: Display memory usage and analysis

## Memory Efficiency

The implementation achieves significant memory reduction:
- Traditional PBFT implementation: ~8.6 MB
- TinyBFT: ~165 KB (reduction of 98.9%)

This efficiency enables BFT replication on tiny devices with as little as 400KB of RAM, opening new possibilities for resilient edge computing, IoT security, and safety-critical embedded systems.

## Acknowledgments

This demo is based on the research paper:
> Harald Böhm, Tobias Distler, Peter Wägemann. "TinyBFT: Byzantine Fault-Tolerant Replication for Highly Resource-Constrained Embedded Systems." IEEE Real-Time and Embedded Technology and Applications Symposium (RTAS), 2024.
