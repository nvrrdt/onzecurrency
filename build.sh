#!/bin/bash

if [-d ./build]; then
  cd ./build;
  cmake .. && make && ./crowd;
else
  mkdir -p ./build;
  cd ./build;
  cmake .. && make && ./crowd;
fi