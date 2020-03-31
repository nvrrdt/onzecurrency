# 1) copy the source to a folder
# 2) remove the block, remove build folder and adapt CMakeLists.txt
# 3) build original system and copied system
# 4) verify test
# 5) cleanup copied folder

import os
import shutil
from pathlib import Path
from subprocess import call
import threading

# make a copy of the crowd folder in the crowd folder
def copy_source(crowd_path, test_path):
    try:
        shutil.copytree(crowd_path, test_path)
    # Directories are the same
    except shutil.Error as e:
        print('Directory not copied. Error: %s' % e)
    # Any error saying that the directory doesn't exist
    except OSError as e:
        print('Directory not copied. Error: %s' % e)

# remove contents of blockchain folder in the copy of crowd
def empty_blockchain_folder(root_path):
    p = Path(root_path / "blockchain")

    for root, dirs, files in os.walk(p):
        for f in files:
            os.unlink(os.path.join(root, f))
        for d in dirs:
            shutil.rmtree(os.path.join(root, d))

# the build folder needs to be rebuild in the building stage, so delete it
def delete_build_folder(test_path):
    p = Path(test_path / "build")
    
    try:
        shutil.rmtree(p)
    # Directory doesn't exist
    except shutil.Error as e:
        print('Directory not deleted. Error: %s' % e)
    # Any error saying that the directory doesn't exist
    except OSError as e:
        print('Directory not deleted. Error: %s' % e)

# CMakeLists.txt needs to be adapted to a new executable object, so that OS can run it
def adapt_cmakeliststxt_test(test_path, test_folder):
    clt = Path(test_path / "CMakeLists.txt")

    # Read in the file
    with open(clt, 'r') as file :
        filedata = file.read()

    # Replace the target string
    filedata = filedata.replace('crowd', test_folder)

    # Write the file out again
    with open(clt, 'w') as file:
        file.write(filedata)

# building and testing stage of 2 executable crowd objects
def build_and_test(crowd_path, test_path):
    crowd_build = Path(crowd_path / "scripts/build.sh")
    test_build = Path(test_path / "scripts/build.sh")

    try:
        r = threading.Thread(target=call_script, args=(crowd_build, ))
        s = threading.Thread(target=call_script, args=(test_build, ))
        r.start()
        s.start()
        r.join()
        s.join()

        input("Continue? [enter]: ") # wasn't able to continue this function after the threads were finished
    except:
        print("Error: unable to start thread")

# call_script used in building and testing stage
def call_script(script_path):
    with open(script_path, 'rb') as file:
        script = file.read()

    command = ['gnome-terminal','--', 'bash', '-c', script]
    call(command, shell=False, env=os.environ.copy())

# delete the testing directory
def cleanup(test_path):
    try:
        shutil.rmtree(test_path)
    # Any error saying that the directory doesn't exist
    except OSError as e:
        print('Directory not deleted. Error: %s' % e)

def main():
    test_folder = "crowd_test1"

    # find the path of the crowd and the test folder
    os.chdir(os.path.dirname(__file__))
    crowd_path = Path(os.getcwd()).parent
    test_path = Path(crowd_path / test_folder)

    # tasks to do
    copy_source(crowd_path, test_path)
    empty_blockchain_folder(crowd_path)
    empty_blockchain_folder(test_path)
    delete_build_folder(test_path)
    adapt_cmakeliststxt_test(test_path, test_folder)
    build_and_test(crowd_path, test_path) # if there are more than 1 test directories, you need to introcude a variadic function
    cleanup(test_path)

main()