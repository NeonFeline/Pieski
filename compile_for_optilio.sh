#!/bin/bash
docker run --platform=linux/amd64 --rm -v $(pwd):/src -w /src gcc:latest g++ -std=c++23 -O3 -o $2 $1
echo "saved compiled" $1 "with name" $2