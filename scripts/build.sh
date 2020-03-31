#!/bin/bash

if [ -d ../build ]; then
  cd ../build;
  cmake -DCMAKE_BUILD_TYPE=Debug .. && make && ./crowd;
else
  mkdir -p ../build;
  cd ../build;
  cmake .. && make && ./crowd;
fi