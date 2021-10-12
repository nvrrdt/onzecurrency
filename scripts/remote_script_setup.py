import sys
import os
import subprocess

def main():
    subprocess.call('echo "it works" && sudo apt install libboost-all-dev librocksdb-dev libenet-dev', shell=True)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
