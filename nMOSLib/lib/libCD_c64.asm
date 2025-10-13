
;**********
;           C64
;**********

;	................................................................	Address

.weak

	TARGET						= 	TARGET_C64
	
	SCREEN						= 	SCREEN_ADDRESS_C64
	COLOR						= 	COLOR_ADDRESS_C64
	SCREEN_WIDTH				=	SCREEN_WIDTH_C64
	SCREEN_HEIGHT				=	SCREEN_HEIGHT_C64
	SCREEN_SIZE					=	SCREEN_SIZE_C64
	BITMAP						= 	BITMAP_ADDRESS_C64
	
	BASIC_START					=   BASIC_START_C64
	BASIC_STOP					=   BASIC_STOP_C64

	MEM_BOTTOM					=	MEM_BOTTOM_C64
	MEM_TOP						=	MEM_TOP_C64
	MEM_EXTRA_BOTTOM			=	MEM_EXTRA_BOTTOM_C64
	MEM_EXTRA_TOP				=	MEM_EXTRA_TOP_C64

	PROGRAM_STACK_SIZE    		=	STACK_SIZE_SHORT		;    $ff
	PROGRAM_STACK_ADDRESS     	=	MEM_EXTRA_TOP			;    $cfff	

	CHARACTER					=	CHARACTER_ADDRESS_C64
	
	BUFFER_ADDRESS				=	BUFFER_ADDRESS_C64
	BUFFER_SIZE					=	BUFFER_SIZE_C64
	
	HEAP						=   MEM_TOP
	
.endweak

	; FMOD !
	
	basic_store_fac1 				= $BBD4		; FACMEM: store FAC1 to mem, X=low Y=high
	basic_copy_fac2_to_fac1 		= $BBFC		; ARGFAC: copy FAC2 to FAC1
	basic_load5_fac1 				= $BBA2		; MEMFAC: load to FAC1, A=low Y=high
	basic_load5_fac2 				= $BA8C		; MEMARG: load to FAC2, A=low Y=high
	basicROM_fdivt 					= $BB12		; FDIVT: FAC1 = FAC2 / FAC1
	basicROM_int 					= $BCCC		; INT: FAC1 = INT(FAC1)
	basicROM_fmultt 				= $BA2B		; FMULTT: FAC1 = FAC1 * FAC2
	basicROM_fsubt 					= $B853		; FSUBT: FAC1 = FAC2 - FAC1
	
;	................................................................	include

.include "c64/libKERNEL_c64.asm"
.include "c64/libBASIC_c64.asm"
.include "libSTDIO.asm"
.include "libMATH.asm"
.include "libMEM.asm"
.include "libFSTACK.asm"


