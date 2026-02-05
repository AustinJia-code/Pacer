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
In separate terminals:                                                         \
```./sender [bind port] [dest port]```
```./emulator [recv bind] [ack bind] [receiver port] [sender port] [hazard]```
```./receiver [bind port] [ack dest port]```

Example:       
``` bash                                                                    
./sender 9000 9001
./emulator 9001 9002 9003 9000 random-loss
./receiver 9003 9002                                                                 
```

    SENDER               EMULATOR              RECEIVER
  bind: 9000            bind: 9001            bind: 9003
                        bind: 9002             
Data:
    [9000] ---to 9001---> [9001]
                          [9002] ---to 9003---> [9003]

ACKs:
    [9000] <--from 9001-- [9001]
                          [9002] <--from 9003-- [9003]