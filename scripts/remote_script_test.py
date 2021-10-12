import sys
import os
import subprocess

def main():
    subprocess.call('echo it works && clear && echo -e "\033c\e[3J"' \
        ' &&  dpkg -i ./onzecurrency-0.1.1-Linux.deb && apt-get -f install' \
        ' && rm -rf /onzecurrency/.config' \
        ' && onze-terminal', shell=True)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
