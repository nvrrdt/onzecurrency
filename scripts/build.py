import sys
import os
import subprocess
from pathlib import Path

def build_no_arg():
    # find the path of the build folder
    os.chdir(os.path.dirname(__file__))
    build = Path(os.getcwd()).parent / "build"
    
    if os.path.exists(str(build)):
        print(str(build))
        command = ['cd ' + str(build) + ' && cmake -DCMAKE_BUILD_TYPE=Debug .. && make']
        subprocess.call(command, shell=True)
    else:
        os.makedirs(build)
        command = ['cd ' + str(build) + ' && cmake -DCMAKE_BUILD_TYPE=Debug .. && make']
        subprocess.call(command, shell=True)

def build_one_arg(test_path, test_name):
    if os.path.exists(test_path + "/build"):
        command = ['cd ' + test_path + '/build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make']
        subprocess.call(command, shell=True)
    else:
        os.makedirs(test_path + "/build")
        command = ['cd ' + test_path + '/build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make']
        subprocess.call(command, shell=True)

def main():
    if len(sys.argv) > 2:
        print("Max 1 argument ...")
    elif len(sys.argv) == 2:
        print("One argument.")
        test_path = str(sys.argv[1])
        test_name = os.path.basename(test_path)
        build_one_arg(test_path, test_name)
    elif len(sys.argv) == 1:
        print("No argument.")
        build_no_arg()
    else:
        print("There's something wrong!")

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)