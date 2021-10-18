import sys
import os
import subprocess
import argparse
import time
import psutil

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

    # Start the command when synchronised with other servers
    start_test_time = (args.order - 1) * args.block_creation_delay
    time.sleep(start_test_time)
    
    # Run command
    command = 'dpkg -i ./onzecurrency-0.1.1-Linux.deb' \
              '&& apt-get -f install' \
              '&& rm -rf /onzecurrency/.config' \
              '&& echo er@er.c0 | onze-terminal'
    subp = subprocess.Popen(command, shell=True)

    # Let the onze-terminal process exist until al servers have finished
    total_test_time = (args.total_servers - args.order + 1) * args.block_creation_delay
    time.sleep(total_test_time)

    # Kill onze-terminal    
    for proc in psutil.process_iter(attrs=['pid', 'name']):
        if 'onze-terminal' in proc.info['name']:
            proc.kill()

    # future: Scp log files and last blocknumber to tester's main machine

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
