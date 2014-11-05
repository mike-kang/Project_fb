#!/bin/sh

MACHINE=`uname -m`

LD_LIBRARY_PATH=../camera:../inih_r29/arch/${MACHINE}/:../tools/:../ ./main
