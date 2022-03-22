import sys
import os, glob
import subprocess
import argparse
import time
from tokenize import String
import psutil
import pexpect
import threading
import json

from datetime import datetime

def main():
    # Cd to scripts directory
    #os.chdir(os.path.dirname(__file__))

    # Initialize parser
    parser = argparse.ArgumentParser()
    
    # Adding optional arguments
    parser.add_argument("order", help = "Order of server", type=int)
    parser.add_argument("total_servers", help = "Total amount of servers", type=int)
    parser.add_argument("block_creation_delay", help = "Block creation delay", type=int)
    # test1 crowd coin: delete start kill restart   = 1
    # test3 coin: delete start                      = 3
    # test4 coin: start                             = 4
    parser.add_argument("test_nr", help = "Number of test to execute", type=int)

    # Read arguments from command line
    args = parser.parse_args()

    # Install onze-terminal and so
    thread = threading.Thread(target=worker_install)
    thread.start()

    # Delete config folder and its contents
    subprocess.call('rm -rf /onzecurrency/.config', shell=True)

    time.sleep(10) # estimated time to install the software, then start onze-terminal simultaneously over cloud servers

    # Start the command when synchronised with other servers
    start_test_time = 0 if (args.order == 1) else ((args.order - 1) * args.block_creation_delay) - 20
    time.sleep(start_test_time)

    #print("begin", args.order, ",",args.total_servers, ",", datetime.utcnow())

    # Execute onze-terminal --> start with intro_peer
    command = 'onze-terminal'
    child = pexpect.spawn(command, encoding='utf-8', timeout=5)
    child.logfile = sys.stdout
    child.setecho(False)
    child.expect("Email adress: ")
    child.send('er@er.c0\n')

    # Let the onze-terminal process exist until al servers have finished
    remaining_test_time = ((args.total_servers * args.block_creation_delay + 1) + 15) if (args.order == 1) else (((args.total_servers - args.order + 1) * args.block_creation_delay) + 20 + 15)
    time.sleep(remaining_test_time)

    # Test_time2 is killing (and restarting) each onze-terminal with 10 seconds in between to see if network is being informed in both cases
    start_test_time2 = (args.order - 1) * 10
    time.sleep(start_test_time2)
    child.sendcontrol('c') # ctrl-c
    child.close()

    #print("1st ctrl-c", args.order, ",",args.total_servers, ",", datetime.utcnow())

    # Restarting the onze-terminals after 5 seconds
    time.sleep(5)
    child = pexpect.spawn(command, encoding='utf-8', timeout=60)
    child.logfile = sys.stdout
    child.setecho(False)

    remaining_test_time2 = ((args.total_servers - args.order + 1) * 10) - 5
    time.sleep(remaining_test_time2)

    count_blocks()
    
    if args.test_nr == 1:
        # Get all the my_full_hashes
        my_full_hashes = get_full_hashes()

        if args.order <= len(my_full_hashes):
            print("begin", args.order, ",",args.total_servers, ",", datetime.utcnow())

            child.expect("Tx to: ")

            # Sleep the execute the transactions in order
            start_test_time = (args.order - 1) * 5
            time.sleep(start_test_time)

            child.sendline(my_full_hashes[args.order])

            # Let the onze-terminal process exist until al servers have finished
            remaining_test_time = (args.total_servers - args.order + 1) * 5
            time.sleep(remaining_test_time)

            # Ctrl-c
            child.sendcontrol('c') # ctrl-c

            print("ctrl-c", args.order, ",",args.total_servers, ",", datetime.utcnow())
    
    child.close()

def count_blocks():
    # Create file that contains amount of blocks present in log folder
    dir = '/onzecurrency/.config/onzehub/blockchain/crowd/'
    if os.path.exists(dir):
        list = os.listdir(dir) # dir is your directory path
        blocks_count = len(list)
        with open('/onzecurrency/.config/onzehub/log/blocks_count', 'w') as file:
            file.write(str(blocks_count))
        file.close()

def get_full_hashes():
    # search for full_hashes and send them to execute a test

    full_hashes = []
    for file in glob.glob("/onzecurrency/.config/onzehub/blockchain/crowd/*"):
        with open(file, 'r') as f:
            block_j = json.load(f)
            for entry in block_j["entry"]:
                full_hashes.append(entry["full_hash"])
        f.close()

    return full_hashes

def worker_install():
    subprocess.call('dpkg -i ./onzecurrency-0.1.1-Linux.deb && apt-get -f install', shell=True)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
