

; .............. commodore 64

; .............. compile : sh casm.sh tst/libMATH/math007_c64

.include "../../lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../../lib/libCD_c64.asm"

kLocalAsmReal00                     .byte  $81,$33,$33,$33,$33	;	// 1.4
kLocalAsmReal01                     .byte  $82,$4c,$cc,$cc,$cc	;   // 3.2
kLocalAsmReal03                   	.byte  $8b,$00,$18,$93,$74 	;  //  1024.768
kLocalAsmReal04                   	.byte  $8b,$80,$18,$93,$74 	;  // -1024.768
message								.null  "the end"


;	*		u8	u16	s8	s16	f40
;	u8		x	V	V	V	V		
;	u16		V	x	V	V	V	
;	s8		V	V	x	V	V	
;	s16		V	V	V	x	V	
;	f40		V	V	V	V	x	

.comment
	; a
	sty zpy
	ldx zpy
	ldy #0
	;lda #$ff
	;ldx #$ff
	;ldy #$00
	sec
	jsr basic.set_mantissa  ; sets mantissa to 00yyxxaa
	jsr basic.set_rest_fac1 ; set rest of FAC1 and JMP to $b8d2
.endc
	
main .proc

	; .................................... test


	load_imm_ay 65534
	; OK
	;.cast_from_u16_to_f40
	; OK
	jsr basic.conv_u16_to_fac1
	
	jsr basic.print_fac1
	lda #petscii.nl
	jsr kernel.chrout


	rts
	
.endproc

