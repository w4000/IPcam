#!/bin/sh

PS1="\$PWD# "

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/lib:/home/lib2
export PATH=/home/bin:/home/edvr:$PATH

alias ll='ls -alF'
