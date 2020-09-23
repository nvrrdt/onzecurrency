import sys
import os
import subprocess
from pathlib import Path

def main():
    # find the path of the build folder
    os.chdir(os.path.dirname(__file__))
    build_path = str(Path(os.getcwd()).parent / "build")

    command = ['cd ' + build_path + ' \
            && [ -f ./CMakeCache.txt ] \
            && rm CMakeCache.txt \
            || cmake -DCMAKE_BUILD_TYPE=Debug .. \
            && make \
            && ./src/libcrowd/tests --log_level=message \
            && ./src/liblogin/tests --log_level=message \
            && ./src/ui/terminal/onzehub-terminal']

    if os.path.exists(build_path):
        subprocess.call(command, shell=True)
    else:
        os.makedirs(build_path)
        subprocess.call(command, shell=True)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)