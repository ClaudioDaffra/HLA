

; .............. commodore 64

; .............. compile : sh casm.sh tst/libMATH/math000_c64

.include "../../lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../../lib/libCD_c64.asm"

kLocalAsmReal00                     .byte  $81,$33,$33,$33,$33	;	// 1.4
kLocalAsmReal01                     .byte  $82,$4c,$cc,$cc,$cc	;   // 3.2
kLocalAsmReal03                   	.byte  $8b,$00,$18,$93,$74 	;  //  1024.768
kLocalAsmReal04                   	.byte  $8b,$80,$18,$93,$74 	;  // -1024.768
message								.null  "the end"



main .proc
.comment
	lda #8
	math_neg_s8
	jsr std.print_s8_dec

	lda	#petscii.space
	jsr kernel.chrout

	lda #-8
	math_neg_u8
	jsr std.print_u8_dec

	lda	#petscii.space
	jsr kernel.chrout

	load_imm_ay 258
	jsr math.neg_u16
	jsr std.print_s16_dec

	lda	#petscii.space
	jsr kernel.chrout

	load_imm_ay -258
	jsr math.neg_s16
	jsr std.print_u16_dec

	lda	#petscii.space
	jsr kernel.chrout

	load_fac1 kLocalAsmReal00
	jsr math.neg_f40
	jsr std.print_fac1

	lda	#petscii.space
	jsr kernel.chrout
	
	lda #1
	jsr math.not_u8
	jsr std.print_u8_dec

	lda #-1
	jsr math.not_s8
	jsr std.print_u8_dec

	lda	#petscii.nl
	jsr kernel.chrout
	
	load_fac1 math.zero			;	math.zero 0.0
	jsr math.not_f40
	jsr std.print_u8_dec

	load_fac1 kLocalAsmReal00	;	kLocalAsmReal04 -1024.768
	jsr math.not_f40
	jsr std.print_u8_dec
.endc
	;	-------------------------------------	0.76

	lda	#petscii.nl
	jsr kernel.chrout
	
	; 1024.768
	load_fac1 kLocalAsmReal03
	; 3.2
	load_fac2 kLocalAsmReal01

	jsr math.mod_f40
	
	jsr std.print_fac1
	
	;	-------------------------------------
	
	rts
	
.endproc

