import sys
import os
import subprocess
import argparse
import time
import psutil
import pexpect
import threading

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

    # Read arguments from command line
    args = parser.parse_args()

    # Install onze-terminal and so
    thread = threading.Thread(target=worker)
    thread.start()
    time.sleep(6) # estimated time to install the software    

    # Start the command when synchronised with other servers
    start_test_time = ((args.order - 1) * args.block_creation_delay) if (args.order == 1) else ((args.order - 1) * args.block_creation_delay) - 20
    time.sleep(start_test_time)
    
    # Let the onze-terminal process exist until al servers have finished
    remaining_test_time = (((args.total_servers - args.order + 2) * args.block_creation_delay) + 15) if (args.order == 1) else (((args.total_servers - args.order + 2) * args.block_creation_delay) + 20 + 15)
    
    # Execute onze-terminal
    command = 'onze-terminal'
    child = pexpect.spawn(command, encoding='utf-8', timeout=remaining_test_time)
    child.logfile = sys.stdout
    child.setecho(False)
    child.expect("Email adress: ")
    child.send('er@er.c0\n')

    time.sleep(remaining_test_time)

    print(args.order, ",",args.total_servers, ",", datetime.utcnow())

    # Kill onze-terminal    
    for proc in psutil.process_iter(attrs=['pid', 'name']):
        if 'onze-terminal' in proc.info['name']:
            proc.kill()

    # Create file that contains amount of blocks present in log folder
    dir = '/onzecurrency/.config/onzehub/blockchain/crowd/'
    if os.path.exists(dir):
        list = os.listdir(dir) # dir is your directory path
        blocks_count = len(list)
        with open('/onzecurrency/.config/onzehub/log/blocks_count', 'w') as file:
            file.write(str(blocks_count))
        file.close()

def worker():
    subprocess.call('dpkg -i ./onzecurrency-0.1.1-Linux.deb' \
                    ' && apt-get -f install' \
                    ' && rm -rf /onzecurrency/.config', shell=True)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
