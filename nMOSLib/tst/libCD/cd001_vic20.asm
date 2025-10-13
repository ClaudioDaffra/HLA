
; .............. commodore vic20

; .............. compile : sh casm.sh tst/libCD/cd001_vic20

.include "../../lib/libCD.asm"

PROGRAM TARGET_VIC20, PROGRAM_ADDRESS_VIC20, 2025

.include "../../lib/libCD_vic20.asm"


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



