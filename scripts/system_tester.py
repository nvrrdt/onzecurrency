# What to do:
# - Scp the binaries and so to approx. 20 servers
# - Let the binaries run through ssh
# - After a certain delay --> send all the logs + amount of blocks to the tester's main machine through scp

# At the main machine analyze the logs with analyzer.py



# bash but should do: ssh user@remote.host nohup python scriptname.py &
# nohup and you can execute that script on the remote host

import sys
import os, glob
import subprocess
from pathlib import Path
import queue, threading
import time
import shutil
import argparse

def main():
    # Cd to script directory
    os.chdir(os.path.dirname(__file__))

    shutil.rmtree('../log', ignore_errors=True)

    # Variables
    ips = ["51.158.68.232", "51.15.226.67", "51.15.248.67", "212.47.254.170", "212.47.234.94", "212.47.236.102"]
    block_creation_delay = 20 # seconds
    
    # Constant
    string = 'er@er.c0' # username

    # Initialize parser
    parser = argparse.ArgumentParser()
    
    # Adding optional arguments
    parser.add_argument("-s", "--scp-binaries", help = "Scp binaries and so", action="store_true")
    
    # Read arguments from command line
    args = parser.parse_args()
    
    # Do not use ELIF, combining options doesn't work then
    if args.scp_binaries:
        # Set unique username in each server's remote_script_test.py
        for ip in ips:
            with open('remote_script_test.py', 'r') as file :
                filedata = file.read()
            filedata = filedata.replace(string, string[:7] + str(ips.index(ip)) + string[7 + 1:])
            string = string[:7] + str(ips.index(ip)) + string[7 + 1:]
            with open('remote_script_test.py', 'w') as file:
                file.write(filedata)
            file.close()

            subprocess.call('cd ' + project_path("build") + \
                ' && scp `find . -maxdepth 1 -type f -name *.deb` root@' + ip + ':~/' + \
                ' && cd ' + project_path("scripts") + \
                ' && scp remote_script_test.py root@' + ip + ':~/', shell=True)

        with open('remote_script_test.py', 'r') as file :
            filedata = file.read()
        filedata = filedata.replace(string, 'er@er.c0')
        with open('remote_script_test.py', 'w') as file:
            file.write(filedata)
        file.close()

    # Queue to use in the threads
    q = queue.Queue()
    for ip in ips:
        q.put(ip)

    # Creating a thread for each server
    threads = [ threading.Thread(target=worker, args=(q, len(ips), block_creation_delay)) for _i in range(len(ips)) ]
    for thread in threads:
        thread.start()

    # Cd to log directory
    if not os.path.exists("../log"):
        os.makedirs("../log")
    os.chdir("../log")

    # Wait for all of them to finish
    for x in threads:
        x.join()

    # Notify the user
    subprocess.call('notify-send -t 2000 "Done"', shell=True)

    for ip in ips:
        # Scp log files to main machine
        subprocess.call('scp -r root@' + ip + ':/onzecurrency/.config/onzehub/log ..', shell=True)

        # Add ip adress and index to beginning of log file
        new_loggi = "{index}_{ip}_loggi".format(index=ips.index(ip), ip=ip)
        os.rename('loggi', new_loggi)

        # Add ip adress and index to beginning of count file
        if os.path.isfile('blocks_count'):
            new_blocks_count = "{index}_{ip}_blocks_count".format(index=ips.index(ip), ip=ip)
            os.rename('blocks_count', new_blocks_count)

        # Scp blocks to main machine
        subprocess.call('scp -r root@' + ip + ':/onzecurrency/.config/onzehub/blockchain/crowd/ .', shell=True)

        for file in glob.glob("crowd/block*"):
            # Add ip adress and index to beginning of block/file
            f = file.split('/')[1]
            new_f = "crowd/{index}_{ip}_{f}".format(index=ips.index(ip), ip=ip, f=f)
            os.rename(file, new_f)

def worker(q, total_servers, block_creation_delay):
    ip = q.get()
    order = total_servers - q.qsize()
    subprocess.call('ssh root@' + ip + ' python3 remote_script_test.py ' + str(order) + ' ' + str(total_servers) + ' ' + str(block_creation_delay) \
                    + ' && exit', shell=True)
    return

def project_path(sub_dir):
    # find the path of the sub_dir folder in the project folder
    full_path = str(Path(os.getcwd()).parent / sub_dir)

    if not os.path.exists(full_path):
        os.makedirs(full_path)
    
    return full_path

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)