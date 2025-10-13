
; compile : sh casm.sh tst/libKERNEL/kernel000

.include "../../lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../../lib/libCD_c64.asm"

main .proc

	lda #'@'
	jsr kernel.chrout
	
	rts
	
.endproc

