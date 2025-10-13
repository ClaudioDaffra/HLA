@echo off
setlocal
cls
REM Pulizia dei file temporanei
echo
echo --------------------------
echo Cleaning temporary files...
echo --------------------------
del *.pp 2>nul
del *.s 2>nul
del *.prg 2>nul
del *.debug 2>nul

REM Controllo che sia stato passato un parametro
if "%1"=="" (
    echo -------------------------------
    echo Error: No file specified
    echo Usage: %0 filename
    echo -------------------------------
    exit /b 1
)

REM Verifica esistenza file di input per il primo comando
if not exist "%1.hla" (
    echo -----------------------------------------
    echo Error: Input file not found: %1.hla
    echo -----------------------------------------
    exit /b 1
)

REM Esecuzione primo comando (mcpp)
echo Running preprocessor...
node ../bin/node_run_mcpp.js -W 0 -C -e utf8 -2 -k -i %1.hla -o %1.hla.pp
if errorlevel 1 (
    echo ---------------------------------------------
    echo Error during preprocessor execution
    echo ---------------------------------------------
    exit /b 1
)

REM Verifica esistenza file di input per il secondo comando
if not exist "%1.hla.pp" (
    echo ---------------------------------------------------------------
    echo Error: Preprocessor output file not found: %1.hla.pp
    echo ---------------------------------------------------------------
    exit /b 1
)

REM Esecuzione secondo comando (hla)
echo Running HLA compiler...
node ../bin/node_run_hla.js -mcpp -i %1.hla.pp -o %1.s
if errorlevel 1 (
    echo ----------------------------------
    echo Error during HLA compilation
    echo ----------------------------------
    exit /b 1
)

REM Verifica esistenza file di input per il terzo comando
if not exist "%1.s" (
    echo ---------------------------------------
    echo Error: Assembly file not found: %1.s
    echo ---------------------------------------
    exit /b 1
)

REM Esecuzione terzo comando (64tass)
echo Running 64tass assembler...
node ../bin/node_run_64tass.js -C -a -B %1.s -o %1.prg
if errorlevel 1 (
    echo -----------------------------
    echo Error during assembly
    echo -----------------------------
    exit /b 1
)

REM Verifica esistenza file finale
if not exist "%1.prg" (
    echo -------------------------------------------------
    echo Error: Final output file not found: %1.prg
    echo -------------------------------------------------
    exit /b 1
)
echo -------------------------------------------------
echo Compilation completed successfully: %1.prg
echo -------------------------------------------------
exit /b 0
