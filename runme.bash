#!/bin/bash
clear

gcc -O2 pacman.c -lpthread -lncurses

time ./a.out

echo Test Ended.