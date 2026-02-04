# UDP-Based Custom Transport with Link Emulator

Emulates and benchmarks network hazards between a sender and receiver using a
custom UDP-based transport protocol. 

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
