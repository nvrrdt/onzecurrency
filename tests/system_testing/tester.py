# What to do:
# - Scp the binaries and so to approx. 20 servers
# - Let the binaries run through ssh
# - After a certain delay --> send all the logs + amount of blocks to the tester's main machine through scp

# At the main machine analyze the logs with analyzer.py



# bash but should do: ssh user@remote.host nohup python scriptname.py &
# nohup and you can execute that script on the remote host