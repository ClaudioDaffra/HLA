

; .............. commodore 64

; .............. compile : sh casm.sh tst/libMATH/math001_c64

.include "../../lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../../lib/libCD_c64.asm"

kLocalAsmReal00                     .byte  $81,$33,$33,$33,$33	;	// 1.4
kLocalAsmReal01                     .byte  $82,$4c,$cc,$cc,$cc	;   // 3.2
kLocalAsmReal03                   	.byte  $8b,$00,$18,$93,$74 	;  //  1024.768
kLocalAsmReal04                   	.byte  $8b,$80,$18,$93,$74 	;  // -1024.768
message								.null  "the end"


	
main .proc

	; .................................... add u16

	load_imm_zpWord0 		5678
	load_imm_ay 			1234
	jsr math.add_u16
	jsr std.print_u16_dec
	
	lda	#petscii.space
	jsr kernel.chrout

	; .................................... add s16

	load_imm_zpWord0 		-5678
	load_imm_ay 			-1
	jsr math.add_s16
	jsr std.print_s16_dec
	
	lda	#petscii.space
	jsr kernel.chrout

	; .................................... add f40

	load_fac1	kLocalAsmReal00
	load_fac2	kLocalAsmReal01
	jsr math.add_f40
	jsr std.print_fac1

	lda	#petscii.space
	jsr kernel.chrout

	; .................................... mul bytes ay

	lda #8
	ldy #7
	jsr math.mul_u8
	jsr std.print_u8_dec

	lda	#petscii.space
	jsr kernel.chrout
	
	; .................................... mul u8 -> ay

	lda #16
	ldy #31
	jsr math.mul_u8
	jsr std.print_u16_dec

	lda	#petscii.space
	jsr kernel.chrout

	; .................................... mul s8 -> ay

	lda #8
	ldy #-87
	jsr math.mul_s8
	jsr std.print_s16_dec

	lda	#petscii.space
	jsr kernel.chrout

	; .................................... mul f40

	load_fac1	kLocalAsmReal00
	load_fac2	kLocalAsmReal01
	jsr math.mul_f40
	jsr std.print_fac1

	lda	#petscii.nl
	jsr kernel.chrout

	; .................................... mul u16 ay zpWord0 -> ay -> zpDWord1

	load_imm_zpWord0 1234
	load_imm_ay 	 5	
	jsr math.mul_u16	;	6170
	
	;lda zpDWord1+0		exit default ay
	;ldy zpDWord1+1
	jsr std.print_u16_dec

	lda	#petscii.space
	jsr kernel.chrout

	; .................................... mul s16 ay zpWord0 -> ay -> zpDWord1

	load_imm_zpWord0 1234
	load_imm_ay 	 -5	
	jsr math.mul_s16	;	-6170
	
	;lda zpDWord1+0		exit default ay
	;ldy zpDWord1+1
	jsr std.print_s16_dec

	lda	#petscii.nl
	jsr kernel.chrout

	; .................................... div u8 a/y a:y

	lda #56
	ldy #8
	jsr math.div_u8
	jsr std.print_u8_dec
	lda	#petscii.space
	jsr kernel.chrout
	
	; .................................... div s8 a/y a:y

	lda #56
	ldy #-8
	jsr math.div_s8
	jsr std.print_s8_dec
	lda	#petscii.space
	jsr kernel.chrout

	; .................................... div u16 zpWord0 / ay := ay

	load_imm_zpWord0 1234
	load_imm_ay 	 5	
	jsr math.div_u16	;	
	jsr std.print_u16_dec

	lda	#petscii.space
	jsr kernel.chrout

	; .................................... div s16 zpWord0 / ay := ay

	load_imm_zpWord0 -12345
	load_imm_ay 	 1
	jsr math.div_s16	;

	jsr std.print_s16_dec

	lda	#petscii.nl
	jsr kernel.chrout

	; .................................... div f40 fac1/fac2 or fac2/fac1
	
	; div default fac2/fac1 : math.div_f40.fac2_div_by_fac1
	
	; here human redable
	
	load_fac1	kLocalAsmReal01	;	3.2
	load_fac2	kLocalAsmReal00	;	1.4
	jsr math.div_f40			;	3.2 / 1.4
	jsr std.print_fac1

	lda	#petscii.nl
	jsr kernel.chrout
	
	; .................................... mod u8 a % y := a
	
	lda #8
	ldy #3
	jsr math.mod_u8			;	8 % 3 = 2
	jsr std.print_u8_dec
	lda	#petscii.space
	jsr kernel.chrout
	
	; .................................... mod s8 a % y := a
	
	lda #8
	ldy #-3
	jsr math.mod_s8			;	8 % -3 = 2
	jsr std.print_s8_dec
	lda	#petscii.space
	jsr kernel.chrout

	; .................................... mod u16 zpWord0 % ay := ay
	
	load_imm_zpWord0 123
	load_imm_ay 	 22
	jsr math.mod_u16			
	jsr std.print_u16_dec
	lda	#petscii.space
	jsr kernel.chrout
	
	; .................................... mod s16 zpWord0 % ay := ay
	
	load_imm_zpWord0 123
	load_imm_ay 	 -22
	jsr math.mod_s16			
	jsr std.print_u16_dec
	lda	#petscii.nl
	jsr kernel.chrout

	; .................................... mod fac1

	; 1024.768 % 3.2 := 0.768
	;					0.685
	
	load_fac1	kLocalAsmReal03	;	1024.768
	load_fac2	kLocalAsmReal01	;	3.2
	jsr math.mod_f40

	jsr std.print_fac1

	; .................................... sub signed 16

	load_imm_zpWord0 -12345
	load_imm_ay 	 1
	jsr math.sub_s16
	
	jsr std.print_s16_dec

	lda	#petscii.nl
	jsr kernel.chrout

	; .................................... sub fac1 - fac2

	load_fac1	kLocalAsmReal03	;	1024.768
	load_fac2	kLocalAsmReal01	;	3.2
	jsr math.sub_f40

	jsr std.print_fac1

	lda	#petscii.nl
	jsr kernel.chrout
	
	; .................................... neg byte

	lda #8
	math_neg_u8
	jsr std.print_s8_dec
	lda	#petscii.nl
	jsr kernel.chrout
	
	lda #-8
	math_neg_u8
	jsr std.print_u8_dec
	lda	#petscii.nl
	jsr kernel.chrout

	load_imm_ay 32767
	jsr math.neg_u16
	jsr std.print_s16_dec
	lda	#petscii.nl
	jsr kernel.chrout
	
	load_imm_ay -32767
	jsr math.neg_s16
	jsr std.print_u16_dec
	lda	#petscii.nl
	jsr kernel.chrout
	
	;

	; .................................... mul f40
	
	;	1024.768
	load_fac1	kLocalAsmReal03
	;   3.2
	load_fac2	kLocalAsmReal01
	
	jsr math.mod_f40

	jsr std.print_fac1

	lda	#petscii.nl
	jsr kernel.chrout
	
	rts
	
.endproc

