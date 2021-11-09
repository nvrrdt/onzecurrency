# Follow every step every server's log has gone through
# Formulate a conclusion

# Also count blocks

# a search function, a 1 file assembler of all log files

import os, glob, sys
import argparse
import string

def main():
    # Initialize parser
    parser = argparse.ArgumentParser()
    
    # Adding optional arguments
    parser.add_argument("-a", "--assemble", help = "Assemble 1 file of all log files", action="store_true")
    parser.add_argument("-s", "--search", help = "Search in log files", type=str, action="store")
    
    # Read arguments from command line
    args = parser.parse_args()
    
    # Do not use ELIF, combining options doesn't work then
    if args.assemble:
        assemble()
    if args.search:
        search(args.search)

# Create one_loggi file from all log files
def assemble():
    messages = []
    folder_path = 'log'
    for filename in glob.glob(os.path.join(folder_path, '*')):
        if filename == 'log/one_loggi':
            continue
        with open(filename, 'r') as f:
            lines = f.readlines()
            for line in lines:
                date, time, severity, function_name, message = line.split(' ', maxsplit=4)
                updated_line = [date, time, filename , severity, function_name, message]
                messages.append(' '.join(updated_line))
        f.close()

    messages.sort()

    with open('log/one_loggi', 'w') as f:
        for message in messages:
            f.writelines(message)

# Search for a string in all log files
def search(search_term):
    messages = []
    folder_path = 'log'
    for filename in glob.glob(os.path.join(folder_path, '*')):
        if filename == 'log/one_loggi':
            continue
        with open(filename, 'r') as f:
            lines = f.readlines()
            for line in lines:
                date, time, severity, function_name, message = line.split(' ', maxsplit=4)
                updated_line = [date, time, filename , severity, function_name, message]
                if search_term in message:
                    messages.append(' '.join(updated_line))
        f.close()
    
    messages.sort()

    for message in messages:
        print(message)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)