FROM ubuntu:focal
WORKDIR /libcrowd
COPY . .
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y git g++ cmake wget python3-virtualenv libboost-all-dev libssl-dev \
                                                                        libcurl4-openssl-dev libleveldb-dev libminiupnpc-dev libsnappy-dev
RUN virtualenv -p /usr/bin/python3 .venv && . ./.venv/bin/activate
RUN rm ./build/CMakeCache.txt
CMD [ "python3", "./scripts/build.py" ]