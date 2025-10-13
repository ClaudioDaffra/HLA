

;	**************		
;
;	cpu, encoding
;
;	**************	

CPU6502	.macro
	.cpu  '6502'
	.enc  'none'
.endmacro

;	**************		
;
;	TARGET
;
;	**************	

	TARGET_C64					= 	0
	TARGET_VIC20				= 	1
	TARGET_VIC20_3K				= 	2
	TARGET_VIC20_8K				= 	3
	TARGET_VIC20_16K			= 	4
	TARGET_VIC20_24K			= 	5
	TARGET_VIC20_32K			= 	6
	TARGET_C128					= 	7
	
	STACK_SIZE_SHORT			=	 255
	STACK_SIZE_LONG				=	1024
	
	;	.............................................. c64
	
	PROGRAM_ADDRESS_C64			=	$0801
	SCREEN_ADDRESS_C64			=	$0400
	SCREEN_WIDTH_C64			=	40
	SCREEN_HEIGHT_C64			=	25
	SCREEN_SIZE_C64				=   999
	COLOR_ADDRESS_C64			=	$D800	
	
	BITMAP_ADDRESS_C64			=	$2000
	BASIC_START_C64				=	PROGRAM_ADDRESS_C64
	BASIC_STOP_C64				=	$9fff
	MEM_BOTTOM_C64				=	BASIC_START_C64
	MEM_TOP_C64					=	BASIC_STOP_C64
	MEM_EXTRA_BOTTOM_C64		=	$c000
	MEM_EXTRA_TOP_C64			=	$cfff
	CHARACTER_ADDRESS_C64		=	53248 ; $d000
	BUFFER_ADDRESS_C64			=	679
	BUFFER_SIZE_C64				=	88
	
	;	.............................................. vic20
	
	SCREEN_WIDTH_VIC20			=	22
	SCREEN_HEIGHT_VIC20			=	22
	PROGRAM_ADDRESS_VIC20		=	4097	;	$1001
	SCREEN_ADDRESS_VIC20		=	7680	;	$1e00
	SCREEN_SIZE_VIC20			=	505
	COLOR_ADDRESS_VIC20			=	38400	;	$9600	
	BITMAP_ADDRESS_VIC20		=	SCREEN_ADDRESS_VIC20
	
	BASIC_START_VIC20			=	PROGRAM_ADDRESS_VIC20
	BASIC_STOP_VIC20			=	7679	;	$1DFF
	MEM_BOTTOM_VIC20			=	BASIC_START_VIC20
	MEM_TOP_VIC20				=	BASIC_STOP_VIC20
	MEM_EXTRA_BOTTOM_VIC20		=	MEM_BOTTOM_VIC20
	MEM_EXTRA_TOP_VIC20			=	MEM_TOP_VIC20
	CHARACTER_ADDRESS_VIC20		=	32768
	BUFFER_ADDRESS_VIC20		=	679
	BUFFER_SIZE_VIC20			=	88
	
	;	.............................................. VIC20_3K

	SCREEN_WIDTH_VIC20_3K		=	22
	SCREEN_HEIGHT_VIC20_3K		=	22
	PROGRAM_ADDRESS_VIC20_3K	=	1025	;	$0401
	SCREEN_ADDRESS_VIC20_3K		=	7680	;	$1e00
	SCREEN_SIZE_VIC20_3K		=	505
	COLOR_ADDRESS_VIC20_3K		=	38400	;	$9600	
	BITMAP_ADDRESS_VIC20_3K		=	SCREEN_ADDRESS_VIC20_3K
	
	BASIC_START_VIC20_3K		=	PROGRAM_ADDRESS_VIC20_3K
	BASIC_STOP_VIC20_3k			=	7679	;	$1DFF
	MEM_BOTTOM_VIC20_3K			=	BASIC_START_VIC20_3K
	MEM_TOP_VIC20_3K			=	BASIC_STOP_VIC20_3k
	MEM_EXTRA_BOTTOM_VIC20_3K	=	MEM_BOTTOM_VIC20_3K
	MEM_EXTRA_TOP_VIC20_3K		=	MEM_TOP_VIC20_3K
	CHARACTER_ADDRESS_VIC20_3K	=	32768
	BUFFER_ADDRESS_VIC20_3K		=	679
	BUFFER_SIZE_VIC20_3K		=	88
	
	;	.............................................. VIC20_8K

	SCREEN_WIDTH_VIC20_8K		=	22
	SCREEN_HEIGHT_VIC20_8K		=	22
	PROGRAM_ADDRESS_VIC20_8K	=	4609	;	$1201
	SCREEN_ADDRESS_VIC20_8K		=	4096	;	$1000
	SCREEN_SIZE_VIC20_8K		=	505
	COLOR_ADDRESS_VIC20_8K		=	37888	;	$9400	
	BITMAP_ADDRESS_VIC20_8K		=	SCREEN_ADDRESS_VIC20_8K
	
	BASIC_START_VIC20_8K		=	PROGRAM_ADDRESS_VIC20_8K
	BASIC_STOP_VIC20_8K			=	16384	;	4000
	MEM_BOTTOM_VIC20_8K			=	BASIC_START_VIC20_8K
	MEM_TOP_VIC20_8K			=	BASIC_STOP_VIC20_8k
	MEM_EXTRA_BOTTOM_VIC20_8K	=	MEM_BOTTOM_VIC20_8K
	MEM_EXTRA_TOP_VIC20_8K		=	MEM_TOP_VIC2_8K
	CHARACTER_ADDRESS_VIC20_8K	=	32768
	BUFFER_ADDRESS_VIC20_8K		=	679
	BUFFER_SIZE_VIC20_8K		=	88
	
	;	.............................................. vic16k

	SCREEN_WIDTH_VIC20_16K		=	22
	SCREEN_HEIGHT_VIC20_16K		=	22
	PROGRAM_ADDRESS_VIC20_16K	=	4609	;	$1201
	SCREEN_ADDRESS_VIC20_16K	=	4096	;	$1000
	SCREEN_SIZE_VIC20_16K		=	505
	COLOR_ADDRESS_VIC20_16K		=	37888	;	$9400	
	BITMAP_ADDRESS_VIC_16K		=	SCREEN_ADDRESS_VIC20_16K
	
	BASIC_START_VIC20_16K		=	PROGRAM_ADDRESS_VIC20_16K
	BASIC_STOP_VIC20_16k		=	24576	;	$6000	
	MEM_BOTTOM_VIC20_16K		=	BASIC_START_VIC20_16K
	MEM_TOP_VIC20_16K			=	BASIC_STOP_VIC20_16k
	MEM_EXTRA_BOTTOM_VIC20_16K	=	MEM_BOTTOM_VIC20_16K
	MEM_EXTRA_TOP_VIC20_16K		=	MEM_TOP_VIC20_16K
	CHARACTER_ADDRESS_VIC20_16K	=	32768
	BUFFER_ADDRESS_VIC20_16K	=	679
	BUFFER_SIZE_VIC20_16K		=	88
	
	;	.............................................. VIC20_24K

	SCREEN_WIDTH_VIC20_24K		=	22
	SCREEN_HEIGHT_VIC20_24K		=	22	
	PROGRAM_ADDRESS_VIC20_24K	=	4609	;	$1201
	SCREEN_ADDRESS_VIC20_24K	=	4096	;	$1000
	SCREEN_SIZE_VIC20_24K		=	505
	COLOR_ADDRESS_VIC20_24K		=	37888	;	$9400	
	BITMAP_ADDRESS_VIC24K		=	SCREEN_ADDRESS_VIC20_24K
	
	BASIC_START_VIC20_24K		=	PROGRAM_ADDRESS_VIC20_24K
	BASIC_STOP_VIC20_24K		=	32768	;	$8000
	MEM_BOTTOM_VIC20_24K		=	BASIC_START_VIC20_24K
	MEM_TOP_VIC20_24K			=	BASIC_STOP_VIC20_24k
	MEM_EXTRA_BOTTOM_VIC20_24K	=	MEM_BOTTOM_VIC20_24K
	MEM_EXTRA_TOP_VIC20_24K		=	MEM_TOP_VIC20_24K
	CHARACTER_ADDRESS_VIC20_24K	=	32768
	BUFFER_ADDRESS_VIC20_24K	=	679
	BUFFER_SIZE_VIC20_24K		=	88
	
	;	.............................................. VIC20_32K

	SCREEN_WIDTH_VIC20_32K		=	22
	SCREEN_HEIGHT_VIC20_32K		=	22	
	PROGRAM_ADDRESS_VIC20_32K	=	4609	;	$1201
	SCREEN_ADDRESS_VIC20_32K	=	4096	;	$1000
	SCREEN_SIZE_VIC20_32K		=	505
	COLOR_ADDRESS_VIC20_32K		=	37888	;	$9400	
	BITMAP_ADDRESS_VIC20_32K	=	SCREEN_ADDRESS_VIC20_32K
	
	BASIC_START_VIC20_32K		=	PROGRAM_ADDRESS_VIC20_32K
	BASIC_STOP_VIC20_32K		=	32768	;	$8000
	MEM_BOTTOM_VIC20_32K		=	BASIC_START_VIC20_32K
	MEM_TOP_VIC20_32K			=	BASIC_STOP_VIC20_32k
	MEM_EXTRA_BOTTOM_VIC20_32K	=	$a000
	MEM_EXTRA_TOP_VIC20_32K		=	$b000
	CHARACTER_ADDRESS_VIC20_32K	=	32768
	BUFFER_ADDRESS_VIC20_32K	=	679
	BUFFER_SIZE_VIC20_32K		=	88
	
	;	.............................................. c128
	
	PROGRAM_ADDRESS_C128		=	$1C01
	SCREEN_ADDRESS_C128			=	$0400
	SCREEN_WIDTH_C128			=	40
	SCREEN_HEIGHT_C128			=	25
	SCREEN_SIZE_C128			=   999
	COLOR_ADDRESS_C128			=	$D800	
	
	BITMAP_ADDRESS_C128			=	$2000
	BASIC_START_C128			=	PROGRAM_ADDRESS_C128
	BASIC_STOP_C128				=	$9fff
	MEM_BOTTOM_C128				=	BASIC_START_C128
	MEM_TOP_C128				=	BASIC_STOP_C128
	MEM_EXTRA_BOTTOM_C128		=	$c000
	MEM_EXTRA_TOP_C128			=	$cfff
	CHARACTER_ADDRESS_C128		=	53248 ; $D000
	BUFFER_ADDRESS_C128			=	$1300
	BUFFER_SIZE_C128			=	$17FF-BUFFER_ADDRESS_C128
	
	;	$1C01 is the normal start address. It gets raised to 
	;	$4001 if a bitmap graphics mode is active.
	
	
;	**************		
;
;	DEFINE
;
;	**************	

;	................................................................	program line

.weak

	PROGRAM_LINE			=	2025
	
.endweak

;	................................................................	END

SIZEOF_BYTE	=	1
SIZEOF_WORD	=	2
SIZEOF_REAL	=	5

;	**************		
;
;	PROGRAM
;
;	**************	

;
;	PROGRAM TARGET_C64 , $0801 , 2024
;	basic program with sys call	

PROGRAM	.macro	_TARGET=TARGET_C64, _BASIC_ADDRESS=PROGRAM_ADDRESS_C64,_LINE=PROGRAM_LINE
.if ((\_TARGET==TARGET_C64)||(\_TARGET==TARGET_VIC20)||(\_TARGET==TARGET_VIC20_3K)||(\_TARGET==TARGET_VIC20_8K)||(\_TARGET==TARGET_VIC20_16K)||(\_TARGET==TARGET_VIC20_24K)||(\_TARGET==TARGET_VIC20_32K)||(\_TARGET==TARGET_C128)) 
*       = \_BASIC_ADDRESS
        .word (+), \_LINE  					;	pointer, line number
        .null $9e, format("%4d", start) 
+       .word 0          					;	basic line end
*       = \_BASIC_ADDRESS+12
.else
.error "?? TARGET non supported" 
.endif
	start:

		jsr fstack.init

		clc
		cld

		jsr main
		
		rts
.endmacro

;	................................................................	GLOBAL MACRO

load_mem_ay	.macro
	lda \1+0
	ldy \1+1
.endmacro

load_imm_ay	.macro
	lda #<\1
	ldy #>\1
.endmacro

load_address_ay	.macro
	lda #<\1
	ldy #>\1
.endmacro

load_addr_ay	.macro
	lda #<\1
	ldy #>\1
.endmacro

load_fac1	.macro
	load_imm_ay \1
	jsr basic.load5_fac1
.endmacro

load_fac2	.macro
	load_imm_ay \1
	jsr basic.load5_fac2
.endmacro

load_addr_zpWord0	.macro
	load_imm_ay \1
	sta zpWord0+0
	sty zpWord0+1
.endmacro

load_addr_zpWord1	.macro
	load_imm_ay \1
	sta zpWord1+0
	sty zpWord1+1
.endmacro

load_imm_zpWord0	.macro
	load_imm_ay \1
	sta zpWord0+0
	sty zpWord0+1
.endmacro

load_imm_zpWord1	.macro
	load_imm_ay \1
	sta zpWord1+0
	sty zpWord1+1
.endmacro

;C2	.macro
;		eor #255
;		clc
;		adc #1
;.endmacro

sev .macro
    sec
    lda #$80    ; set overflow
    sbc #$01
.endm

;	................................................................	pescii

; https://en.wikipedia.org/wiki/PETSCII

petscii .proc 

	nl				=  13
	minuscolo		=  14
	maiuscolo		=  142
	lowercase		=  14
	uppercase		=  142
	clear_screen	=  147
	space			=  32

.endproc

ascii .proc 

	;clear_screen	= 
	
.endproc

color	.proc

	black		=	0
	white		=	1
	red			=	2
	cyan		=	3
	purple		=	4
	green		=	5
	blue		=	6
	yellow		=	7
	
	orange		=	8
	brown		=	9
	pink		=	10
	darkGrey	=	11
	grey		=	12
	lightGrey	=	13
	lightBlue	=	14
	lightGreen	=	15

.endproc

;	**************		
;
;	ZERO PAGE
;
;	**************	

hla_ZeroPAge			=	06
hla_reg0				=	hla_fstack_param+0
hla_reg1				=	hla_fstack_param+1
hla_reg2				=	hla_fstack_param+2
hla_reg3				=	hla_fstack_param+3
hla_reg4				=	hla_fstack_param+4
hla_reg5				=	hla_fstack_param+5
hla_reg6				=	hla_fstack_param+6
hla_reg7				=	hla_fstack_param+7
hla_reg8				=	hla_fstack_param+8
hla_reg9				=	hla_fstack_param+9
hla_reg10				=	hla_fstack_param+10
hla_reg11				=	hla_fstack_param+11
hla_reg12				=	hla_fstack_param+12
hla_reg13				=	hla_fstack_param+13
hla_reg14				=	hla_fstack_param+14
hla_reg15				=	hla_fstack_param+15

zpa			= 02
zpy         = 03
zpx			= 04
tmp			= 05
sp			= 247
bp			= 249

zpDWord0	= $fb	;	251			4 byte

	zpByte0		= zpDWord0+0
	zpByte1		= zpDWord0+1
	zpByte2		= zpDWord0+2
	zpByte3		= zpDWord0+3

	zpWord0     = zpDWord0+0
		zpWord0hi   = zpWord0+0
		zpWord0lo   = zpWord0+1
	zpWord1     = zpDWord0+2
		zpWord1hi   = zpWord1+0
		zpWord1lo   = zpWord1+1

zpDWord1    = $62
    zpWord2     = $62
        zpWord2hi   = $62
        zpWord2lo   = $62+1
    zpWord3     = $64
        zpWord3hi   = $64
        zpWord3lo   = $64+1

; zpDWord2
;
;zpDWord2_addr = $03
;
;zpDWord2    = $03
;    zpWord4     = $03       ;   $B1AA, execution address of routine converting floating point to integer.
;        zpWord4hi   = $03
;        zpWord4lo   = $03+1
;    zpWord5     = $05       ;   $B391, execution address of routine converting integer to floating point.
;        zpWord5hi   = $05
;        zpWord5lo   = $05+1

;	**************		
;
;	LINK
;
;	**************	

; 	https://vic20reloaded.com/commodore-vic-20-memory-map/
;	https://www.zimmers.net/anonftp/pub/cbm/src/vic20/vic20_rom_disassembly.txt


;
;
;


