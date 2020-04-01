import sys
import os
import subprocess

def build_no_arg():
    if os.path.exists("../build"):
        command = ['cd ../build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make && ./crowd && read']
        subprocess.call(command, shell=True)
    else:
        os.makedirs("../build")
        command = ['cd ../build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make && ./crowd && read']
        subprocess.call(command, shell=True)

def build_one_arg(test_path, test_name):
    if os.path.exists(test_path + "/build"):
        command = ['cd ' + test_path + '/build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make && ./' + test_name + ' && read']
        subprocess.call(command, shell=True)
    else:
        os.makedirs(test_path + "/build")
        command = ['cd ' + test_path + '/build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make && ./' + test_name + ' && read']
        subprocess.call(command, shell=True)

def main():
    if len(sys.argv) > 2:
        print("Max 1 argument ...")
    elif len(sys.argv) == 2:
        print("Eén argument.")
        test_path = str(sys.argv[1])
        test_name = os.path.basename(test_path)
        build_one_arg(test_path, test_name)
    elif len(sys.argv) == 1:
        print("Geen argument.")
        build_no_arg()
    else:
        print("There's something wrong!")

main()