#!/bin/bash
clear ; 
echo "Building WASM ..."
rm bin/hla > /dev/null 2>&1
rm bin/hla.js > /dev/null 2>&1
rm bin/hla.wasm > /dev/null 2>&1
cd src 
make -f eMakefile clean 
make -f eMakefile 
mv hla.* ../bin/ 
rm *.o ; 
cd ..

