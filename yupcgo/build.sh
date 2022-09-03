#!/bin/sh

if [ ! -d build ]; then
    mkdir build
fi

go build -x -o bin/yupc main.go
