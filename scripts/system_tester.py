# What to do:
# - Scp the binaries and so to approx. 20 servers
# - Let the binaries run through ssh
# - After a certain delay --> send all the logs + amount of blocks to the tester's main machine through scp

# At the main machine analyze the logs with analyzer.py



# bash but should do: ssh user@remote.host nohup python scriptname.py &
# nohup and you can execute that script on the remote host

import sys
import os
import subprocess
from pathlib import Path
import queue, threading
import time

def main():
    # Cd to script directory
    os.chdir(os.path.dirname(__file__))

    # Variables
    ips = ["51.158.68.232", "51.15.226.67"] # , "51.15.248.67", "212.47.254.170", "212.47.234.94", "212.47.236.102"]
    block_creation_delay = 20 # seconds
    
    # Constant
    string = 'er@er.c0' # username

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

    # Wait until al servers have finished
    total_test_time = ((len(ips) + 1) * block_creation_delay) + 10
    time.sleep(total_test_time)

    # Scp log files to main machine
    for ip in ips:
        subprocess.call('scp -r root@' + ip + ':~/onzecurrency/.config/onzehub/log ./log', shell=True)

def worker(q, total_servers, block_creation_delay):
    ip = q.get()
    order = total_servers - q.qsize()
    subprocess.call('ssh root@' + ip + ' python3 remote_script_test.py ' + str(order) + ' ' + str(total_servers) + ' ' + str(block_creation_delay), shell=True)
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