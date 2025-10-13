#!/bin/sh
rm prj/*.prg > /dev/null 2>&1
rm prj/*.pp > /dev/null 2>&1
rm prj/*.s > /dev/null 2>&1
mcpp -W 0 -C -e utf8 -2 -k $1.hla -o $1.hla.pp
bin/hla -mcpp -t vic20 -i $1.hla.pp -o $1.s

