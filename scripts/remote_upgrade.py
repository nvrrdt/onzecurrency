import sys
import os
import subprocess
import threading
import queue

def main():
    ips = ["212.47.231.236", "51.15.226.67", "51.15.248.67", "212.47.254.170", "212.47.234.94", "212.47.236.102"]
    #ips = ["51.15.248.67"]

    # Queue to use in the threads
    q = queue.Queue()
    for ip in ips:
        q.put(ip)

    # Creating a thread for each server
    threads = [ threading.Thread(target=worker, args=(q,)) for _i in range(len(ips)) ]
    for thread in threads:
        thread.start()

    # Wait for all of them to finish
    for t in threads:
        t.join()

    # Notify the user
    subprocess.call('notify-send -t 2000 "Done"', shell=True)

def worker(q):
    ip = q.get()
    subprocess.call('ssh root@' + ip + ' "apt autoremove -y && apt update && apt upgrade -y && reboot"', shell=True)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)