import sys
import os
import subprocess

def main():
    subprocess.call('dpkg -i ./onzecurrency-0.1.1-Linux.deb && apt-get -f install' \
        ' && rm -rf /onzecurrency/.config' \
        ' && echo er@er.c0 | onze-terminal', shell=True)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
