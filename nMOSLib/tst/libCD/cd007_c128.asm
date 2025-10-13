
; .............. commodore vic20

; .............. compile : sh casm.sh tst/libCD/cd007_c128

.include "../../lib/libCD.asm"

PROGRAM TARGET_C128, PROGRAM_ADDRESS_C128, 2025

.include "../../lib/libCD_c128.asm"


main .proc

	lda	#TARGET
	sta	SCREEN
	lda #color.green
	sta COLOR

	lda	#TARGET
	sta	SCREEN+SCREEN_SIZE
	lda #color.red
	sta COLOR+SCREEN_SIZE
	
	rts
	
.endproc



