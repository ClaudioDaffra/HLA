

; .............. commodore 64

; .............. compile : sh casm.sh tst/libMATH/math003_c64

.include "../../lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../../lib/libCD_c64.asm"



; ---------------------------------------------------------

main .proc

	; .................................... math.mul_by_2	2046

	lda #<1023
	ldy #>1023
	
	#math_mul_word_by_word
	jsr std.print_u16_dec
	
	lda	#petscii.space
	jsr kernel.chrout

	; .................................... math.mul_by_5	5115

	lda #<1023
	ldy #>1023
	
	#math_mul_word_by_real
	jsr std.print_u16_dec
	
	lda	#petscii.space
	jsr kernel.chrout

	; .................................... math.mul_by_type struct::(8) 8184
	; ay := ay * zpWord0
	
	lda #<1023
	ldy #>1023
	ldx #8
	#math_mul_word_by_type
	jsr std.print_u16_dec
	
	lda	#petscii.space
	jsr kernel.chrout
	
	; .................................... math.mul_by_2	46

	lda #<23
	ldy #>23

	#math_mul_word_by_word
	jsr std.print_u16_dec
	
	lda	#petscii.space
	jsr kernel.chrout

	; .................................... math.mul_by_5	115

	lda #<23
	ldy #>23
	
	#math_mul_word_by_real
	jsr std.print_u16_dec
	
	lda	#petscii.space
	jsr kernel.chrout

	; .................................... math.mul_by_type struct::(8) : 184

	lda #<23
	ldy #>23
	ldx #8
	
	#math_mul_word_by_type
	jsr std.print_u16_dec
	
	lda	#petscii.space
	jsr kernel.chrout


	;	-----------------------------------------------	184 , 8184
	lda #<23
	ldy #>23
	ldx #8

	#math_mul_word_by_type
	jsr std.print_u16_dec
	
	lda	#petscii.space
	jsr kernel.chrout

	lda #<1023
	ldy #>1023
	ldx #8

	#math_mul_word_by_type
	jsr std.print_u16_dec
	
	lda	#petscii.space
	jsr kernel.chrout
	
	;	-----------------------------------------------	
	
	rts
	
.endproc

