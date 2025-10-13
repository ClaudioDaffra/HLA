#!/bin/bash

# Pulizia dei file temporanei
echo
echo "--------------------------"
echo "Cleaning temporary files..."
echo "--------------------------"
rm -f *.pp *.s *.prg *.debug

# Controllo che sia stato passato un parametro
if [ -z "$1" ]; then
    echo "-------------------------------"
    echo "Error: No file specified"
    echo "Usage: $0 filename"
    echo "-------------------------------"
    exit 1
fi

# Verifica esistenza file di input per il primo comando
if [ ! -f "$1.hla" ]; then
    echo "-----------------------------------------"
    echo "Error: Input file not found: $1.hla"
    echo "-----------------------------------------"
    exit 1
fi

# Esecuzione primo comando (mcpp)
echo "Running preprocessor..."
node ../bin/node_run_mcpp.js -W 0 -C -e utf8 -2 -k -i "$1.hla" -o "$1.hla.pp"
if [ $? -ne 0 ]; then
    echo "---------------------------------------------"
    echo "Error during preprocessor execution"
    echo "---------------------------------------------"
    exit 1
fi

# Verifica esistenza file di input per il secondo comando
if [ ! -f "$1.hla.pp" ]; then
    echo "---------------------------------------------------------------"
    echo "Error: Preprocessor output file not found: $1.hla.pp"
    echo "---------------------------------------------------------------"
    exit 1
fi

# Esecuzione secondo comando (hla)
echo "Running HLA compiler..."
node ../bin/node_run_hla.js -mcpp -i "$1.hla.pp" -o "$1.s"
if [ $? -ne 0 ]; then
    echo "----------------------------------"
    echo "Error during HLA compilation"
    echo "----------------------------------"
    exit 1
fi

# Verifica esistenza file di input per il terzo comando
if [ ! -f "$1.s" ]; then
    echo "---------------------------------------"
    echo "Error: Assembly file not found: $1.s"
    echo "---------------------------------------"
    exit 1
fi

# Esecuzione terzo comando (64tass)
echo "Running 64tass assembler..."
node ../bin/node_run_64tass.js -C -a -B "$1.s" -o "$1.prg"
if [ $? -ne 0 ]; then
    echo "-----------------------------"
    echo "Error during assembly"
    echo "-----------------------------"
    exit 1
fi

# Verifica esistenza file finale
if [ ! -f "$1.prg" ]; then
    echo "-------------------------------------------------"
    echo "Error: Final output file not found: $1.prg"
    echo "-------------------------------------------------"
    exit 1
fi

echo "-------------------------------------------------"
echo "Compilation completed successfully: $1.prg"
echo "-------------------------------------------------"
exit 0
