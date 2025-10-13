
;**********
;           VIC20_3K
;**********

;	................................................................	Address

.weak

	TARGET						= 	TARGET_VIC20_3K
	
	SCREEN						= 	SCREEN_ADDRESS_VIC20_3K
	COLOR						= 	COLOR_ADDRESS_VIC20_3K
	SCREEN_WIDTH				=	SCREEN_WIDTH_VIC20_3K
	SCREEN_HEIGHT				=	SCREEN_HEIGHT_VIC20_3K
	SCREEN_SIZE					=	SCREEN_SIZE_VIC20_3K
	BITMAP						= 	BITMAP_ADDRESS_VIC20_3K
	
	BASIC_START					=   BASIC_START_VIC20_3K
	BASIC_STOP					=   BASIC_STOP_VIC20_3K

	MEM_BOTTOM					=	MEM_BOTTOM_VIC20_3K
	MEM_TOP						=	MEM_TOP_VIC20_3K
	MEM_EXTRA_BOTTOM			=	MEM_EXTRA_BOTTOM_VIC20_3K
	MEM_EXTRA_TOP				=	MEM_EXTRA_TOP_VIC20_3K

	PROGRAM_STACK_SIZE    		=	STACK_SIZE_SHORT		;	$ff
	PROGRAM_STACK_ADDRESS     	=	MEM_EXTRA_TOP			;	7679	;	$1DFF

	CHARACTER					=	CHARACTER_ADDRESS_VIC20_3K
	
	BUFFER_ADDRESS				=	BUFFER_ADDRESS_VIC20_3K
	BUFFER_SIZE					=	BUFFER_SIZE_VIC20_3K
	
	HEAP						=   MEM_TOP
	
.endweak

	; FMOD !
	
	basic_store_fac1 				= $DBD4		; FACMEM: store FAC1 to mem, X=low Y=high
	basic_copy_fac2_to_fac1 		= $DBFC		; ARGFAC: copy FAC2 to FAC1
	basic_load5_fac1 				= $DBA2		; MEMFAC: load to FAC1, A=low Y=high
	basic_load5_fac2 				= $DA8C		; MEMARG: load to FAC2, A=low Y=high
	basicROM_fdivt 					= $DB12		; FDIVT: FAC1 = FAC2 / FAC1
	basicROM_int 					= $DCCC		; INT: FAC1 = INT(FAC1)
	basicROM_fmultt 				= $DA2B		; FMULTT: FAC1 = FAC1 * FAC2
	basicROM_fsubt 					= $D853		; FSUBT: FAC1 = FAC2 - FAC1
	
;	................................................................	include

.include "vic20/libKERNEL_vic20.asm"
.include "vic20/libBASIC_vic20.asm"
.include "libSTDIO.asm"
.include "libMATH.asm"
.include "libMEM.asm"
.include "libFSTACK.asm"
