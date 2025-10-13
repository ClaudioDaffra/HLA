
;**********
;           C128
;**********

;	................................................................	Address

.weak

	TARGET						= 	TARGET_C128
	
	SCREEN						= 	SCREEN_ADDRESS_C128
	COLOR						= 	COLOR_ADDRESS_C128
	SCREEN_WIDTH				=	SCREEN_WIDTH_C128
	SCREEN_HEIGHT				=	SCREEN_HEIGHT_C128
	SCREEN_SIZE					=	SCREEN_SIZE_C128
	BITMAP						= 	BITMAP_ADDRESS_C128
	
	BASIC_START					=   BASIC_START_C128
	BASIC_STOP					=   BASIC_STOP_C128

	MEM_BOTTOM					=	MEM_BOTTOM_C128
	MEM_TOP						=	MEM_TOP_C128
	MEM_EXTRA_BOTTOM			=	MEM_EXTRA_BOTTOM_C128
	MEM_EXTRA_TOP				=	MEM_EXTRA_TOP_C128

	PROGRAM_STACK_SIZE    		=	STACK_SIZE_SHORT		;	$ff
	PROGRAM_STACK_ADDRESS     	=	MEM_EXTRA_TOP			;	$cfff	

	CHARACTER					=	CHARACTER_ADDRESS_C128
	
	BUFFER_ADDRESS				=	BUFFER_ADDRESS_C128
	BUFFER_SIZE					=	BUFFER_SIZE_C128
	
	HEAP						=   MEM_TOP
			
.endweak


;	................................................................	ROM ON OFF

c128_disable_rom	.macro
    lda $ff00
    pha
    lda #0
    sta $ff00
.endm

c128_enable_rom	.macro	
    pla
    sta $ff00
.endm

;	................................................................	include

.include "c128/libKERNEL_c128.asm"
.include "c128/libBASIC_c128.asm"
.include "libSTDIO.asm"
.include "libMATH_c128.asm"
.include "libMEM.asm"
.include "libFSTACK_c128.asm"
