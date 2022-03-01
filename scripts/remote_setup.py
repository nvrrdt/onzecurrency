import sys
import os
import subprocess

def main():
    #ips = ["212.47.231.236", "51.15.226.67", "51.15.248.67", "212.47.254.170", "212.47.234.94", "212.47.236.102"]
    ips = ["212.47.231.236"]

    for ip in ips:
        subprocess.call('ssh root@' + ip + ' "apt install -y libboost-all-dev librocksdb-dev python3-pip; pip install psutil"', shell=True)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)