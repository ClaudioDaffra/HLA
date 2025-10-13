#!/bin/bash
clear ; 
echo "Cleaning ..."
rm prj/*.pp > /dev/null 2>&1
rm prj/*.debug
rm prj/*.prg
rm prj/*.s
cd src 
make eMakefile clean 
cd ..




