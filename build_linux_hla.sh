#!/bin/bash
clear ; 
echo "Building ..."
rm bin/hla > /dev/null 2>&1
cd src ; 
make clean ; 
make ; 
mv hla ../bin/ ; 
rm *.o ; 
cd ..


