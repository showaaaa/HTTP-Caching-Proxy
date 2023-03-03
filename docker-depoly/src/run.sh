#!/bin/bash
make clean
make
echo 'start running proxy server...'
./proxy &
while true ; do continue ; done
