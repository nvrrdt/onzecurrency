import argparse
import sys
import os
import subprocess
from pathlib import Path

def main():
    # Cd to scripts/build.py directory
    os.chdir(os.path.dirname(__file__))

    # Initialize parser
    parser = argparse.ArgumentParser()
    
    # Adding optional arguments
    parser.add_argument("-r", "--remove-build-dir", help = "Remove build directory", action="store_true")
    parser.add_argument("-m", "--make", help = "Make", action="store_true")
    parser.add_argument("-i", "--install", help = "Make install", action="store_true")
    parser.add_argument("-u", "--uninstall", help = "Make uninstall", action="store_true")
    parser.add_argument("-t", "--tests", help = "Run tests", action="store_true")
    parser.add_argument("-e", "--execute", help = "Execute binary after install", action="store_true")
    
    # Read arguments from command line
    args = parser.parse_args()
    
    # Do not use ELIF, combining options doesn't work then
    if args.remove_build_dir:
        if os.path.exists(project_path("build")):
            subprocess.call('rm -r ' + project_path("build"), shell=True)  # suppose we're in ./scripts directory
    if args.make:
        subprocess.call('cd ' + project_path("build") + ' && cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug .. && ninja', shell=True)
    if args.install:
        subprocess.call('cd ' + project_path("build") + ' && ninja install && chmod +x /usr/local/bin/onze-terminal', shell=True)
    if args.uninstall:
        subprocess.call('cd ' + project_path("build") + ' && xargs rm < install_manifest.txt', shell=True)
    if args.tests:
        subprocess.call('cd ' + project_path("build") + ' && ./src/libcrowd/testscrowd --log_level=message \
            && ./src/liblogin/testslogin --log_level=message', shell=True)
    if args.execute:
        subprocess.call('onze-terminal', shell=True)

def project_path(sub_dir):
    # find the path of the build folder
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