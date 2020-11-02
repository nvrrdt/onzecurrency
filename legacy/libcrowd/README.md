### libcrowd

#### build procedure
- . ../.venv/bin/activate
- python ./scripts/build.py
- ./build/tests --log_level=message

#### docker
- sudo docker-compose up
- sudo docker-compose up --build
- sudo docker image ls
- sudo docker run -it libcrowd_libcrowd /bin/bash

#### git
- git rm --cached -r . # prepare a reset
- git reset --hard HEAD~13 # reset last 13 commmits || git reset –hard <commit-hash>
- git push origin master --force # post reset