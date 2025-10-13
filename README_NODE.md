
-----

### NODE.js

-----

Di seguito facciamo installazione e guida alla compilazione usando wasm, per prima cosa dovete installare ovviamente node.js ( ultima versione )
Se volete ricompilare, il compilatore con wasm installte emscripten inizializzate le variabili ambiente

1) ricordatevi di eseguire :
 
**source emsdk_env.sh**
**source emsdk_env.bat**

```
claudio@LenovoLegion:~/emsdk$ source emsdk_env.sh
Setting up EMSDK environment (suppress these messages with EMSDK_QUIET=1)
Adding directories to PATH:
PATH += /home/claudio/emsdk
PATH += /home/claudio/emsdk/upstream/emscripten

Setting environment variables:
PATH = /home/claudio/emsdk:/home/claudio/emsdk/upstream/emscripten:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/lib/wsl/lib:/mnt/c/Program Files/Python313/Scripts/:/mnt/c/Program Files/Python313/:/mnt/c/WINDOWS/system32:/mnt/c/WINDOWS:/mnt/c/WINDOWS/System32/Wbem:/mnt/c/WINDOWS/System32/WindowsPowerShell/v1.0/:/mnt/c/WINDOWS/System32/OpenSSH/:/mnt/c/Program Files/NVIDIA Corporation/NVIDIA App/NvDLISR:/mnt/c/Program Files (x86)/NVIDIA Corporation/PhysX/Common:/mnt/c/nodejs/:/mnt/c/ProgramData/chocolatey/bin:/mnt/c/FPC/3.2.2/bin/i386-Win32:/mnt/c/Program Files/LLVM/bin:/mnt/c/Program Files/dotnet/:/mnt/c/Program Files (x86)/Windows Kits/10/Windows Performance Toolkit/:/mnt/c/Program Files/Microsoft SQL Server/170/Tools/Binn/:/mnt/c/Program Files/Microsoft SQL Server/Client SDK/ODBC/170/Tools/Binn/:/mnt/c/Program Files/Git/cmd:/mnt/c/Users/claud/AppData/Local/Microsoft/WindowsApps:/mnt/c/Users/claud/AppData/Local/Programs/Microsoft VS Code/bin:/mnt/c/Users/claud/.dotnet/tools:/mnt/c/c64/GTK3VICE-3.9-win64/bin:/mnt/c/c64/64tass-1.60.3243:/mnt/c/c64/oscar64/bin:/mnt/c/c64/cc65-snapshot-win32/bin:/mnt/c/Users/claud/AppData/Roaming/npm:/mnt/c/Users/claud/Downloads/SDL2VICE-3.9-win64/SDL2VICE-3.9-win64:/mnt/c/Users/claud/AppData/Local/atom/bin:/mnt/c/Users/claud/.lmstudio/bin:/mnt/c/c64/mufflon-3.0+GUI-win64:/mnt/c/zproject/Mad-Pascal/bin:/snap/bin
EMSDK = /home/claudio/emsdk
EMSDK_NODE = /home/claudio/emsdk/node/22.16.0_64bit/bin/node
claudio@LenovoLegion:~/emsdk$
```

2) compilazione :

**sh build_wasm.sh**

```
Building WASM ...
--- Pulizia dei file generati... ---
rm -f f40_converter.o compiler.o error.o streamer.o lexer.o parser.o symbolTable.o ast.o genCode.o optimizer.o main.o hla.js *.wasm
Compilando f40_converter.cpp...
em++ -std=c++23 -Wall -pedantic -Wextra -O3 -c f40_converter.cpp -o f40_converter.o
Compilando compiler.cpp...
em++ -std=c++23 -Wall -pedantic -Wextra -O3 -c compiler.cpp -o compiler.o
Compilando error.cpp...
em++ -std=c++23 -Wall -pedantic -Wextra -O3 -c error.cpp -o error.o
Compilando streamer.cpp...
em++ -std=c++23 -Wall -pedantic -Wextra -O3 -c streamer.cpp -o streamer.o
Compilando lexer.cpp...
em++ -std=c++23 -Wall -pedantic -Wextra -O3 -c lexer.cpp -o lexer.o
Compilando parser.cpp...
em++ -std=c++23 -Wall -pedantic -Wextra -O3 -c parser.cpp -o parser.o
Compilando symbolTable.cpp...
em++ -std=c++23 -Wall -pedantic -Wextra -O3 -c symbolTable.cpp -o symbolTable.o
Compilando ast.cpp...
em++ -std=c++23 -Wall -pedantic -Wextra -O3 -c ast.cpp -o ast.o
Compilando genCode.cpp...
em++ -std=c++23 -Wall -pedantic -Wextra -O3 -c genCode.cpp -o genCode.o
Compilando optimizer.cpp...
em++ -std=c++23 -Wall -pedantic -Wextra -O3 -c optimizer.cpp -o optimizer.o
Compilando main.cpp...
em++ -std=c++23 -Wall -pedantic -Wextra -O3 -c main.cpp -o main.o
--- Linking finale: creazione di hla.js e hla.wasm ---
em++ -std=c++23 -Wall -pedantic -Wextra -O3 f40_converter.o compiler.o error.o streamer.o lexer.o parser.o symbolTable.o ast.o genCode.o optimizer.o main.o -s WASM=1 -s ENVIRONMENT=node -s FORCE_FILESYSTEM=1 -s EXPORTED_RUNTIME_METHODS='["FS","callMain"]' -lnodefs.js -o hla.js
--- Build completato con successo ---
claudio@LenovoLegion:~/HLA$
```

**directory : bin**

**directory : prj**

```
cd prj
```

**preprocessore**

```
mcpp -W 0 -C -e utf8 -2 -k prj01.hla -o prj01.hla.pp
```

**compilatore**

```
node ../bin/node_run_hla.js -mcpp -i prj01.hla.pp -o prj01.s
```

**output**



**assembler**



**sh run_wasm.sh**

