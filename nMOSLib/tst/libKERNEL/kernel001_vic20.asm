
; compile : sh casm.sh tst/libKERNEL/kernel001_vic20

.include "../../lib/libCD.asm"

PROGRAM TARGET_VIC20, PROGRAM_ADDRESS_VIC20 , 2025

.include "../../lib/libCD_vic20.asm"

main .proc

	lda #'@'
	jsr kernel.chrout
	
	rts
	
.endproc

