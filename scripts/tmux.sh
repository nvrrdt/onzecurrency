#!/bin/sh

tmux new -s onze -d

tmux selectp -t 0
tmux splitw -v -p 33
tmux selectp -t 0
tmux splitw -v -p 50
tmux selectp -t 0
tmux splitw -h -p 50
tmux selectp -t 2
tmux splitw -h -p 50
tmux selectp -t 4
tmux splitw -h -p 50

tmux selectp -t 0    # go back to the first pane

tmux send-keys -t 0 'ssh root@212.47.231.236' Enter
tmux send-keys -t 1 'ssh root@212.47.254.170' Enter
tmux send-keys -t 2 'ssh root@51.15.226.67' Enter
tmux send-keys -t 3 'ssh root@212.47.234.94' Enter
tmux send-keys -t 4 'ssh root@51.15.248.67' Enter
tmux send-keys -t 5 'ssh root@212.47.236.102' Enter

tmux attach -t onze