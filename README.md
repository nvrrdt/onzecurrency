## onzecurrency

Onzecurrency is a decentralised blockchain with a new kind of consensus algorithm called poco or proof of chosen ones.  
If you like to know the technicalities of poco please read [this paper](papers/onzecurrency_crowd.pdf).  
Compared to proof of work poco is energy friendly, compared to proof of stake poco has a fairer reward system.  
Poco is a type of blockchain that is susceptible to 51% Sybil attacks, compared to the 51% compute attack of proof of work.  
Also poco is normally easier regulatable, while pseudonimity remains possible.

## onze-terminal, libcommon, liblogin, libpoco, libcrowd and libcoin

#### build and test procedure
- docker and docker-compose needs to be installed
- sudo docker-compose run -p 1975:1975/tcp -p 1975:1975/udp onzecurrency bash
- python3 ./scripts/build.py -rcd   # to install cryptopp and enet
- python3 ./scripts/build.py -p     # to build and package the current state of onzecurrency
- rm -rf ./config && onze-terminal  # for running onze-terminal once, see the test procedure below for a group of servers
- python3 ./scripts/build.py -t     # there aren't many tests ...

#### test procedure with 6 servers
- foresee 6 servers (in the cloud)

- in a first terminal: cd to onzecurrency folder; python3 ./scripts/build.py -s   
 = scp packaged deb file to servers, ip addresses should be adapted in ./scripts/build.py
- in a second terminal: cd to onzecurrency folder
- ./scripts/tmux.sh
- ssh into the 6 servers
- dpkg -i ./onzecurrency-0.1.1-Linux.deb        # do in each terminal
- apt-get -f install                            # do in each terminal
- rm -rf /onzecurrency/.config && onze-terminal # do in each terminal  

   Continuation: fill in an email address in the first server (a first/genesis is created then, do an 'ls /onzecurrency/.config/onzehub/blockchain/crowd after the test procedure), then fill in an email address in the second server. Wait 20 seconds (the block creation period) and let then run until '--------5:' appears. Then fill in an email address in the third server, wait 20 seconds, let it run, then fill the email address in the fourth server and let it run. At the fourth server the second server will get its full_hash (== user_id) and a second block will be created (do an ls!). '--------5:' denotes the end of the minig and sifting process. If you then fill in an email address for the fifth server, then the third server will receive its full_hash and a third block will be created.  
This process of adding more servers should be able to continue limitlessly, unfortunately the software isn't yet as stable as should be for this endeavour.  
Adding two or more servers within the block creation period results in adding two new users as an entry in the concerning block.
#### doing a new test procedure on the server
- dpkg -i ./onzecurrency-0.1.1-Linux.deb && apt-get -f install && rm -rf /onzecurrency/.config && onze-terminal  

#### only testing the program
- rm -rf /onzecurrency/.config && onze-terminal

## some commands for easy retrieval:

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