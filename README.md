### onze-terminal, libcommon, liblogin and libcrowd

#### build and test procedure
- docker and docker-compose needs to be installed
- sudo docker-compose run -p 1975:1975/tcp -p 1975:1975/udp onzecurrency bash
- python3 ./scripts/build.py -rcd   # to install cryptopp and enet
- python3 ./scripts/build.py -p     # to build and package the current state of onzecurrency
- rm -rf ./config && onze-terminal
- python3 ./scripts/build.py -t     # there aren't many tests ...

#### test procedure with 4 servers
- foresee 4 servers (in the cloud)

- in a first terminal: cd to onzecurrency folder; python3 ./scripts/build.py -s   
 = scp packaged deb file to servers, ip addresses should be adapted in ./scripts/build.py
- in a second terminal: cd to onzecurrency folder
- ./scripts/tmux.sh
- ssh into the 4 servers
- dpkg -i ./onzecurrency-0.1.1-Linux.deb
- apt-get -f install
- rm -rf /onzecurrency/.config && onze-terminal

#### doing a new test procedure on the server
- dpkg -i ./onzecurrency-0.1.1-Linux.deb && apt-get -f install && rm -rf /onzecurrency/.config && onze-terminal  

#### only testing the program
- rm -rf /onzecurrency/.config && onze-terminal

### some commands for easy retrieval:

#### docker
- sudo docker-compose up
- sudo docker-compose up --build
- sudo docker image ls
- sudo docker run -it libcrowd_libcrowd /bin/bash  # doesn't see files, see next
- sudo docker-compose run -p 1975:1975/tcp -p 1975:1975/udp onzecurrency bash  # to see files in docker-compose volume

#### git
- git rm --cached -r .              # prepare a reset
- git reset --hard HEAD~13          # reset last 13 commmits || git reset –hard <commit-hash>
- git push origin master --force    # post reset

#### blocked port
- sudo docker-compose down
- sudo lsof -i -P -n | grep 1975
- sudo kill -9 <process_id>