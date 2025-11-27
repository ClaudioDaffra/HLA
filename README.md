# High Level Assembler (version 0.1)

## Claudio Daffra

[](LICENSE.md)

---

It is a high-level assembler whose purpose is to introduce into assembly typical structures of more advanced languages.
It sits halfway between traditional assembler and a language like C.

## Installation

HLA requires a C++ compiler compatible with the C++23 standard.
It is also recommended to use MCPP as a preprocessor.
The source code is translated into assembly language, then compiled with 64tass (Turbo Assembler).
Alternatively, it can be used with node.js.

**Linux**: native compilation usage

**Windows**: usage with Node.js (Batch files and wrappers are provided, with precompiled Wasm)

p.s. although the Windows executable compiles correctly without issues, it generates incorrect code.

## Language rules and conventions

1. **Variable shadowing**

   * Nested blocks are not allowed.
   * It is not allowed to declare a variable with the same name as an existing one (no shadowing).
   * This check also applies to global variables: they cannot be redefined or shadowed in local scopes.

2. **Passing parameters and constants**

   * Parameters, including constants, must be passed correctly, respecting type and visibility rules.
   * Postfixes are added, for example: `1u16` indicates a word even if initially it is a byte.

3. **Ternary operator (`? :`)**

   * Use of the ternary operator is not allowed.

4. **Alternative operators for better readability**

   * Some operators have been modified compared to C for greater clarity:

     * `:=`    equivalent to `=`      ->    assignment
     * `?=`    equivalent to `==`     ->    comparison
     * `^`     equivalent to `*`      ->    pointer
     * `@`     equivalent to `&`      ->    address
     * `%%`    equivalent to `%`      ->    modulo
     * `->`    equivalent to `fn ret` ->    function return parameter
       bitwise:
     * `%|`    equivalent to `|`      ->    bitwise or
     * `%&`    equivalent to `&`      ->    bitwise and
     * `%^`    equivalent to `^`      ->    bitwise xor
     * `%~`    equivalent to `~`      ->    bitwise neg
     * `%-`    equivalent to `~`      ->    bitwise neg

## Identifiers

Identifiers cannot begin with the `$` symbol nor contain it.
This symbol is reserved for hexadecimal numbers.

## Keywords

```
    fn     main ( )                          ->    argc and argv are added by default, return u8
    fn     sub1 ( px:u8,py:u16 ) -> u8  { }  ->    standard
    sys    proc1( byte:x,word:ay ) { }       ->    sys calls differ from fn
    ret                                      ->    value return
    if ( ) { } else { } ;                    ->    if then else construct
    jmp label ;                              ->    jumps internally within the function
    label:    ;                              ->    label
    types                                    ->    u8,s8,u16,s16,f40
    ns namespace { } ;                       ->    stdio::print , kernel::chrout
```

## Mathematical operations implemented

0. **Operator precedence** follows the same rules as in C.

1. **Unary operators** (as in C):

   * `!`, `-`, `+`
   * `^`, `@`            Pointer(*) and Address(&)
   * `+`, `-`            and Pointer arithmetic
   * `++`,`--`           Pointer arithmetic PREFIX/POSTFIX (not applicable to f40)
   * `%-`,`%~`           bitwise neg

2. **Binary operators** (as in C, with a change):

   * `*`    ,     `/`      , `%%` (where `%%` equals `%` in C)
   * `+`    ,     `-`
   * `<`    ,     `<=`, `>`, `>=`
   * `?=`   ,     `!=`
   * `:=`   assign
   * `%&`   ,     `%|`     ,  `&^`    not applicable to f40
   * `>>`   ,     `<<`                not applicable to f40
   * `::`                             scope resolution

## Number handling

1. **Supported numeric formats:**

   * **Decimals and reals**: `1`, `1.2`, `1.e10`, `1.2e+10`, `1.2e-10`
   * **Binary (C and C64 style)**: `0b0101`, `%0101`
   * **Hexadecimal (C and C64 style)**: `0xFFFF`, `$FFFF`, `0xFFFFp1`
     ** not compatible with mcpp
   * **Decimals and reals**: `1`, `123'456`
   * **Binary (C and C64 style)**: `0b1010'1010`, `%1010'1010`

## Types

```
    u8,s8,u16,s16,f40    ;
    ^ pointer            ;
    
    x     :     u8       ;
    px    :    ^u8       ;
```

## Constants

```
    ci    :    10;
    cs    :    "string";
    cf    :    1.2
```

## Define

```
    #define ci    10
    #define cs    "string";
    #define cf    1.2
```

## Real number conversion

Real numbers can be converted into the Microsoft 5-byte floating-point format (MBF).

## Literal suffixes

They allow you to specify the type of constants: u8 s8 u16 s16 f40.
Usage (mandatory when passing constants as parameters), especially in syscalls:

```
1u16 (indicates that 1 is an unsigned word)  
1s8  (indicates that 1 is a signed byte)  
```

It also allows forcing the data type, e.g. `-1s16` is a signed 16-bit word.

## Usage

```
High Level Assembler
Claudio Daffra [2025]

Usage: HLA [options] [file...]
Options:
  -i, --input <file>     Specify the input file.
  -o, --output <file>    Specify the output file (default: a.out).
  -g                     Emit debug symbols and streamer file.
  -O                     Enable optimizations.
  -T <width>             Set tab width (default: 4).
  -t <arch>              Specify the target architecture (default: C64).
                         Supported: C64, VIC20, VIC20_3K, VIC20_8K, VIC20_16K,
                         VIC20_24K, VIC20_32K, C128.
  -s <address>           Specify a custom start address (e.g., 49152 or '$c000').
  -mcpp                  Enable compatibility with mcpp (disables apostrophe as number separator).
  -f32tof40 <number>     Convert a 32-bit float  to 5-byte MBF and print it.
  -f64tof40 <number>     Convert a 64-bit double to 5-byte MBF and print it.
  -h, --help             Display this information.
```

Batch (`.bat`) and shell (`.sh`) files are provided to simplify compilation and usage of the assembler.
In Linux, the custom address must be specified with quotes: `'$c000'`, to avoid conflicts with shell variables.

## Debug

With the `-g` option, `.debug` files are generated for each compiler phase.

## Examples

* The **tst/** folder contains some HLA examples.
* The **prj/** folder is also ready for compilation with node.js.

### Library

* The **nHLALib/** folder contains the High Level Assembler library (with namespaces).
* The **nMOSLib/** folder contains the assembler library.

## Heap management

The heap for dynamic allocation starts at the end of the program.
The initial location is: **hla_heap**

## Stack management

The stack is implemented using a 16-bit word, allowing up to 65,536 memory positions.

On the Commodore 64, the stack is currently placed at the end of a 4 KB area, but its position can vary depending on the platform, such as the VIC-20 or others.

Functions can use up to 256 bytes of local variables for parameter passing.

Additionally, the hardware stack is used for expression evaluation.
However, since this stack is shared among routines, it is recommended to break down overly complex expressions to avoid conflicts or overflows. Under normal conditions, the stack capacity should be sufficient.

**String management**

Since local stack space is limited to 256 bytes, strings are placed at the end of the listing as constants and loaded as a pointer to unsigned char: ^u8.
Two identical strings will occupy separate memory space.

## Syscalls

These are more like internal library calls.
Parameter passing differs from function conventions.
Types are generic: byte, word, real.

It is possible to use registers: a, x, y, ay, ax, xy, which must be declared at the end of parameter declarations.

Keywords: **byte**, **word**, **real**

Since syscalls are raw, suffixes must also be specified if missing. ( no recursive )

```
    sys test (byte:a,word:xy,real:fac1,real:r10,byte:r9 )  
    {
        asm { 
            ...
        } ;
    } ;

    fn sub1 ( x:u8,y:u8 )  -> u16
    {
        ret 0 ;
    } ;
    
    fn main ( )  
    {
        test(1,2u16,1.2,3.4,-1);
        ret 0 ;
    } ;
    
    p.s. r9/var must be defined in the ASM library
```

Another example from tst/:

```
    ns system
    {
        sys poke ( word:zpWord0,byte:a )
        {
            asm {
                ldy #0
                sta (zpWord0),y
            } ;
        } ;
    } ;
    ...
    system::poke( 53280,1 ) ; // 1 , $d020 208 32
```

## Preprocessor

HLA is designed to work with MCPP, which also supports UTF-8, or it can be used standalone.

1. With MCPP, apostrophes in numbers are not allowed. Extensions such as `1'000'000` are supported by HLA, but to maintain compatibility with MCPP, use the `-mcpp` flag (enable compatibility mode).

2. If you use the preprocessor, do not use `#` in front of names!!! MCPP interprets it as a directive and does not copy it inside `asm { }`. Instead, if needed, use a dot as prefix, e.g.:

```
    .fast_work_get uword, 
```

## Directory structure

```
    bin            :    executable
    doc            :    additional documentation and development
    mcpp           :    mcpp preprocessor
    nHLALib        :    High Level Assembler library
    nMOSLib        :    assembler library
    prj            :    project source folder
    src            :    source code folder
    tst            :    test and usage examples for HLA
```

In the **prj/** folder you can place sources and compile them with batch files from the root folder.

## Optimizations

```
- constant folding (expressions, if then else)  
```

## TODO

```
- statements: loop / break / continue / for ... next
- structures, unions
- Bit Field
- arrays `[]`
- operators ` . ^+ ^-`
- `memory.alloc` (malloc), `memory.dealloc` (free)
- improve asm {}
- explicit cast  
```

## Advanced TODO

* optimization
* preprocessor
* C64Lib porting
* HLA Lib
* ...

### Linux/Ubuntu

```bash
claudio@claudiointeli7:~/HLA$ cat /etc/os-release
PRETTY_NAME="Ubuntu 24.04.3 LTS"
NAME="Ubuntu"
VERSION_ID="24.04"
VERSION="24.04.3 LTS (Noble Numbat)"
VERSION_CODENAME=noble
ID=ubuntu
ID_LIKE=debian
HOME_URL="https://www.ubuntu.com/"
SUPPORT_URL="https://help.ubuntu.com/"
BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
UBUNTU_CODENAME=noble
LOGO=ubuntu-logo
claudio@claudiointeli7:~/HLA$
```

### Compiler

#### GNU/C++

```bash
claudio@claudiointeli7:~/HLA$ g++ --version
g++ (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
Copyright (C) 2023 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

claudio@claudiointeli7:~/HLA$
```

### MCPP

**Linux**

```bash
claudio@claudiointeli7:~/HLA$ mcpp --version
mcpp: illegal option ---
MCPP V.2.7.2 (2008/11) compiler-independent-build compiled by GCC V.13.2
```

### Assembler

**Linux**

```bash
claudio@claudiointeli7:~/HLA$ 64tass --help
Usage: 64tass [OPTIONS...] SOURCES
64tass Turbo Assembler Macro V1.59.3120
```

### WASM

**Install**

```
sudo apt update
sudo apt upgrade -y
sudo apt install git python3
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
emcc --version
...
chmod +x node_run_hla.js
```

**Usage**

In the **prj/** folder there are already two files 
**node_lin_run_mcppWasm_64TassWasm.sh** or 
**node_win_run_mcppWasm_64TassWasm.bat**  
that wrap the binaries in **bin/**.
Usage: **node_win_run_mcppWasm_64TassWasm.bat prj01**, 
it is possible to use wasm simultaneously with the binary or webassembly-compiled versions of mcpp and 64tass.
Refer to the node.js file.

### Documentation

The **doc/** folder contains a file for Notepad++ (hla.xml).

---

### TO BE CONTINUED!

---

