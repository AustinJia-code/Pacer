#!/bin/bash
# Launch sender, emulator, and receiver
# Usage: ./run.sh [hazard] [--paced]

HAZARD=${1:-random-loss}
SENDER_FLAGS=""
for arg in "$@"; do
    if [ "$arg" = "--paced" ]; then
        SENDER_FLAGS="--paced"
    fi
done

SESSION="pacer"
DIR="$(cd "$(dirname "$0")/build" && pwd)"

tmux kill-session -t $SESSION 2>/dev/null

tmux new-session  -d -s $SESSION -c "$DIR" "./sender 9000 9001 $SENDER_FLAGS"
tmux split-window -v -t $SESSION -c "$DIR" "./emulator 9001 9002 9003 9000 $HAZARD"
tmux split-window -v -t $SESSION -c "$DIR" "./receiver 9003 9002"
tmux select-layout -t $SESSION even-vertical

tmux attach -t $SESSION
