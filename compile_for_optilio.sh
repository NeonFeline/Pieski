#!/bin/bash
docker run --platform=linux/amd64 --rm -v $(pwd):/src -w /src gcc:latest g++ -std=c++23 -O3 -o $1 src/dijkstra2.cpp
echo "saved compiled file with name" $1