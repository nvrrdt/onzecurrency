import pexpect
import sys
import time

def expect_one_arg(test_path):
    command = "python ./build.py " + test_path
    child = pexpect.spawn(command, encoding='utf-8')
    child.logfile = sys.stdout
    child.setecho(False)
    child.expect("Network: ")
    child.send('ls\n')
    child.expect("Email adress: ")
    child.send('ls\n')
    child.expect("Password: ")
    child.send('ls\n')
    child.read()


def main():
    if len(sys.argv) > 2:
        print("Max 1 argument ...")
    elif len(sys.argv) == 2:
        print("One argument.")
        test_path = str(sys.argv[1])
        expect_one_arg(test_path)
    elif len(sys.argv) == 1:
        print("Aty least one argument!")
    else:
        print("There's something wrong!")

if __name__ == "__main__":
    main()