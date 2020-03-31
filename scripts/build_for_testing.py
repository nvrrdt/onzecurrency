import pexpect
import sys

child = pexpect.spawn('bash ./build.sh', encoding='utf-8')
child.logfile = sys.stdout
child.setecho(False)
child.expect("Network: ")
child.send('ls\n')
child.expect("Email adress: ")
child.send('ls\n')
child.expect("Password: ")
child.send('ls\n')
child.read()