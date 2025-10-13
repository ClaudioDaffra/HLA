
; .............. commodore vic20

; .............. compile : sh casm.sh tst/libCD/cd005_vic20_24k

.include "../../lib/libCD.asm"

PROGRAM TARGET_VIC20_24K, PROGRAM_ADDRESS_VIC20_24K, 2025

.include "../../lib/libCD_vic20_24k.asm"


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

