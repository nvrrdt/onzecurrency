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

def main():
    # Cd to scripts/build.py directory
    os.chdir(os.path.dirname(__file__))

    ips = ["51.158.68.232", "51.15.226.67", "51.15.248.67", "212.47.254.170", "212.47.234.94", "212.47.236.102"]
    
    for ip in ips:
        subprocess.call('cd ' + project_path("build") + \
            ' && scp `find . -maxdepth 1 -type f -name *.deb` root@' + ip + ':~/' + \
            ' && cd ' + project_path("scripts") + \
            ' && ssh root@' + ip + ' nohup python3 remote_script_test.py', shell=True)

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