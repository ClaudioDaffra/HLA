#!/bin/bash
clear ; 
echo "Cleaning ..."
rm bin/hla > /dev/null 2>&1
rm prj/*.pp > /dev/null 2>&1
rm prj/*.debug
rm prj/*.prg
rm prj/*.s
cd src 
make clean 
cd ..




