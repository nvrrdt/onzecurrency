### libcrowd

#### build procedure
- . ../.venv/bin/activate
- python ./scripts/build.py
- ./build/tests --log_level=message

#### docker
- sudo docker-compose up
- sudo docker-compose up --build
- sudo docker run -it libcrowd_libcrowd /bin/bash