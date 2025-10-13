

; .............. commodore 64

; .............. compile : sh casm.sh tst/libBASIC/basic001_c64

.include "../../lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../../lib/libCD_c64.asm"

kLocalAsmReal00                     .byte  $81,$33,$33,$33,$33	;	// 1.4
kLocalAsmReal01                     .byte  $82,$4c,$cc,$cc,$cc	;   // 3.2
kLocalAsmReal03                   	.byte  $8b,$00,$18,$93,$74 	;  //  1024.768
kLocalAsmReal04                   	.byte  $8b,$80,$18,$93,$74 	;  // -1024.768
message								.null	"the end"

main .proc

	;	neg
	
	load_fac1	kLocalAsmReal00
	jsr basic.neg_fac1
	jsr basic.print_fac1
	
	lda #petscii.nl
	jsr kernel.chrout

	;	add

	load_fac2	kLocalAsmReal00
	load_fac1	kLocalAsmReal00
	jsr basic.add_fac1
	jsr basic.print_fac1

	;	sub

	load_fac2	kLocalAsmReal00
	load_fac1	kLocalAsmReal00
	jsr basic.sub_fac1
	jsr basic.print_fac1

	;	mul

	load_fac2	kLocalAsmReal00
	load_fac1	kLocalAsmReal00
	jsr basic.mul_fac1
	jsr basic.print_fac1

	;	div		fac2 / fac1	:=	3.2 / 1.4

	load_fac2	kLocalAsmReal01
	load_fac1	kLocalAsmReal00
	jsr basic.div_fac1
	jsr basic.print_fac1
	
	; not

	lda #petscii.nl
	jsr kernel.chrout
	
	; 1.4
	load_fac1	kLocalAsmReal00
	jsr basic.not_fac1
	jsr basic.print_unsigned_char
	; 0

	load_fac1	basic.zero
	jsr basic.not_fac1
	;jsr basic.sgn_fac1_to_a
	jsr basic.print_unsigned_char
	
	;
	
	lda #petscii.nl
	jsr kernel.chrout
	
	load_fac1	kLocalAsmReal04
	jsr basic.sgn_fac1_to_a
	jsr basic.print_unsigned_char
	
	; 255
	
	rts
	
	;
	
.endproc

