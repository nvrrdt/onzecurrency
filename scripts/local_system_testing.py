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
import time

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
def adapt_cmakeliststxt(test_path, exe_name):
    clt = Path(test_path / "CMakeLists.txt")

    # Read in the file
    with open(clt, 'r') as file :
        filedata = file.read()

    # Replace the target string
    filedata = filedata.replace('crowd', exe_name)

    # Write the file out again
    with open(clt, 'w') as file:
        file.write(filedata)

# building and testing stage of 2 executable crowd objects
def build_and_test(test_path, position):
    try:
        s = threading.Thread(target=call_script, args=(test_path, position, ))
        s.start()
        time.sleep(2)
        s.join()

    except:
        print("Error: unable to start thread")

# call_script used in building and testing stage
def call_script(test_path, position):
    script_path = Path(test_path / "scripts/send_to_cin.py")

    python_script = "python " + str(script_path) + " " + str(test_path) + " && read"
    command = ['gnome-terminal', '--geometry', position, '--', 'bash', '-c', python_script]
    call(command, shell=False, env=os.environ.copy())

# delete the testing directory
def cleanup(test_path):
    try:
        shutil.rmtree(test_path)
    # Any error saying that the directory doesn't exist
    except OSError as e:
        print('Directory not deleted. Error: %s' % e)

def main():
    tests_folder = [["crowd_test1", "80x24+0+0"], ["crowd_test2", "80x24+660+0"]]

    # find the path of the crowd and the test folder
    os.chdir(os.path.dirname(__file__))
    crowd_path = Path(os.getcwd()).parent

    # tasks to do
    for tf in tests_folder:
        test_path = Path(crowd_path / tf[0])

        copy_source(crowd_path, test_path)
        empty_blockchain_folder(test_path)
        delete_build_folder(test_path)
        adapt_cmakeliststxt(test_path, tf[0])
        build_and_test(test_path, tf[1])
    
    input("type in enter ")

    # cleanup
    for tf in tests_folder:
        test_path = Path(crowd_path / tf[0])

        cleanup(test_path)

main()