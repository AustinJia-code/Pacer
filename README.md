# Pacer
UDP-based transport and network emulator for studying packet pacing, burst loss,
and recovery under high-latency directional links.

# Build:
```bash
cd build
cmake ..
make
```

# Run:
In separate terminals:
Runner: ```./runner [port]```
Receiver: ```./receiver [port]```
Emulator: ```./emulator [receiving port] [sending port]```
