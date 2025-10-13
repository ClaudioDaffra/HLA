
; .............. compile : sh casm.sh tst/libMATH/math002_vic20

.include "../../lib/libCD.asm"

PROGRAM TARGET_VIC20, PROGRAM_ADDRESS_VIC20 , 2025

.include "../../lib/libCD_vic20.asm"

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
	
main .proc

	; .................................... mul unsigned byte fast	; 	56

	lda #8
	ldy #7
	jsr math.mul_u8_fast
	jsr std.print_u16_dec
	
	lda	#petscii.space
	jsr kernel.chrout

	; .................................... mul signed byte fast		;	-56

	lda #8
	ldy #-7
	jsr math.mul_s8_fast
	jsr std.print_s16_dec
	
	lda	#petscii.space
	jsr kernel.chrout

	; .................................... mul unsigned word fast	;	5535

	load_imm_zpWord0 		123
	load_imm_ay 			45
	jsr math.mul_u16_fast
	jsr std.print_u16_dec
	
	lda	#petscii.space
	jsr kernel.chrout

	; .................................... mul signed word fast		;	-5535

	load_imm_zpWord0 		123
	load_imm_ay 			-45
	jsr math.mul_s16_fast
	jsr std.print_s16_dec
	
	lda	#petscii.space
	jsr kernel.chrout

	; .................................... conv string and print	;	-1024.
	
	load_fac1	kLocalAsmReal04
	jsr math.f40_to_string
	jsr std.print_string
	lda #petscii.nl
	jsr kernel.chrout
	
	; .................................... cast u8					

	lda #3					;	3
	cast_from_u8_to_u16
	jsr std.print_u16_dec
	lda #petscii.space
	jsr kernel.chrout

	lda #128				; -128
	cast_from_u8_to_s8
	jsr std.print_s8_dec
	lda #petscii.space
	jsr kernel.chrout

	lda #5					;	5
	cast_from_u8_to_s16
	jsr std.print_s16_dec
	lda #petscii.space
	jsr kernel.chrout

	lda #128				;	128
	cast_from_u8_to_s16
	jsr std.print_s16_dec
	lda #petscii.space
	jsr kernel.chrout

	lda #255				;	255
	cast_from_u8_to_f40
	jsr basic.print_fac1
	lda #petscii.nl
	jsr kernel.chrout
	
	; .................................... cast u16

	load_imm_ay 32767		;	255
	cast_from_u16_to_u8
	jsr std.print_u8_dec
	lda #petscii.space
	jsr kernel.chrout

	load_imm_ay 32767		;	-1
	cast_from_u16_to_s8
	jsr std.print_s8_dec
	lda #petscii.space
	jsr kernel.chrout

	load_imm_ay -32767		;	32767
	cast_from_u16_to_s16
	jsr std.print_s16_dec
	lda #petscii.space
	jsr kernel.chrout

	load_imm_ay 65534
	cast_from_u16_to_f40
	jsr basic.print_fac1
	lda #petscii.nl
	jsr kernel.chrout

	; .................................... cast s8

	lda #128				; -128
	cast_from_s8_to_u8
	jsr std.print_s8_dec
	lda #petscii.space
	jsr kernel.chrout

	lda #3					;	3
	cast_from_s8_to_u16
	jsr std.print_u16_dec
	lda #petscii.space
	jsr kernel.chrout

	lda #128				;	128
	cast_from_s8_to_s16
	jsr std.print_s16_dec
	lda #petscii.space
	jsr kernel.chrout

	lda #-127			    ; -127
	cast_from_s8_to_f40
	jsr basic.print_fac1
	lda #petscii.nl
	jsr kernel.chrout
	
	; .................................... cast s16

	load_imm_ay 32767		;	255
	cast_from_s16_to_u8
	jsr std.print_u8_dec
	lda #petscii.space
	jsr kernel.chrout

	load_imm_ay 32767		;	-1
	cast_from_s16_to_s8
	jsr std.print_s8_dec
	lda #petscii.space
	jsr kernel.chrout

	load_imm_ay -32767		;	32767
	cast_from_s16_to_u16
	jsr std.print_u16_dec
	lda #petscii.space
	jsr kernel.chrout

	load_imm_ay -32767		;	-32767
	cast_from_s16_to_f40
	jsr basic.print_fac1
	lda #petscii.nl
	jsr kernel.chrout

	
	; .................................... cast f40

	load_fac1	kLocalAsmReal00	;	1.4
	cast_from_f40_to_u8
	jsr std.print_u8_dec
	lda #petscii.space
	jsr kernel.chrout

	load_fac1	kLocalAsmReal01	;	3.2
	cast_from_f40_to_s8
	jsr std.print_s8_dec
	lda #petscii.space
	jsr kernel.chrout

	load_fac1	kLocalAsmReal03 ; 	1024.768
	cast_from_f40_to_u16
	jsr std.print_u16_dec
	lda #petscii.space
	jsr kernel.chrout

	load_fac1	kLocalAsmReal04	;	-1024.768
	cast_from_f40_to_s16
	jsr std.print_s16_dec
	lda #petscii.space
	jsr kernel.chrout

	rts
	
.endproc


