# Pacer
UDP-based transport and network emulator for studying packet pacing, burst loss,
and recovery under high-latency directional links.

### Build:
```bash
cd build
cmake ..
make
```

### Run:
In separate terminals:                                                         \
```./receiver [bind port] [ack dest port]```                                   \
```./emulator [recv bind] [ack bind] [receiver port] [sender port] [hazard]``` \
```./sender [bind port] [dest port]```

Example:       
``` bash                                                                    
./receiver 9003 9002      
./emulator 9001 9002 9003 9000 random-loss
./sender 9000 9001
```
```mermaid
%%{init: {'theme':'dark', 'themeVariables': { 'primaryColor':'#1e3a5f','primaryTextColor':'#fff','primaryBorderColor':'#4a90e2','lineColor':'#4a90e2','secondaryColor':'#2d5986','tertiaryColor':'#1a1a2e'}}}%%

graph LR
    subgraph SENDER
        S["<b>bind: 9000</b><br/>[9000]"]
    end
    
    subgraph EMULATOR
        E1["<b>bind: 9001</b><br/>[9001]"]
        E2["<b>bind: 9002</b><br/>[9002]"]
    end
    
    subgraph RECEIVER
        R["<b>bind: 9003</b><br/>[9003]"]
    end
    
    S ===>|"Data<br/>to 9001"| E1
    E2 ===>|"Data<br/>to 9003"| R
    
    E1 -. "ACK<br/>from 9001" .-> S
    R -. "ACK<br/>from 9003" .-> E2

    classDef senderStyle fill:#2d5986,stroke:#4a90e2,stroke-width:3px,color:#fff
    classDef emulatorStyle fill:#1e3a5f,stroke:#4a90e2,stroke-width:3px,color:#fff
    classDef receiverStyle fill:#2d5986,stroke:#4a90e2,stroke-width:3px,color:#fff
    
    class S senderStyle
    class E1,E2 emulatorStyle
    class R receiverStyle
```
