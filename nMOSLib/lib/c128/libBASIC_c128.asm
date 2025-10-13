
;**********
;           BASIC
;**********

; libBASIC C128

basic .proc

	; **********
	;     PRINT
	; **********
	
	.weak
		_print_unsigned_integer = $8E32    ; C128 BASIC - print unsigned integer (low in A, high in X)
		_print_string 			= $55e2    ; Chiama STROUT: stampa la stringa da (Y,A)
	.endweak

	; .................................................................... print_unsigned_integer

	print_unsigned_integer .proc      ; ay -> xa (low in A, high in Y)
		sta zpa
		sty zpy
		.c128_disable_rom
		ldx zpa
		lda zpy
		jsr basic._print_unsigned_integer
		.c128_enable_rom
		rts
	.endproc
		
	; .................................................................... print_unsigned_char
	
	print_unsigned_char .proc         ; a
		sta zpa
		.c128_disable_rom
		lda #0                        ; X = 0 (high)
		ldx zpa
		jsr basic._print_unsigned_integer
		.c128_enable_rom
		rts
	.endproc

	; .................................................................... print_fac1
	
	print_fac1 .proc
		lda $ff
		pha
		.c128_disable_rom
		jsr basic._conv_fac1_to_string
		.c128_enable_rom		
		pla
		sta $ff
		lda #<$0100
		ldy #>$0100
		jsr basic._print_string
		rts
	.endproc

	; **********
	; *    CONV
	; **********
	
	; .................................................................... conv_fac1_to_string
	
	.weak
		_conv_fac1_to_string 	= $8e42		; Chiama AY2ASC: converti FAC1 in stringa ASCII (output a $0100)
		_conv_u8_to_fac1 		= $84D4		; C128 BASIC byte to float
			;Xconv_s8_to_fac1 		= $8815		; C128 BASIC signed fixed to float
			;X_conv_s16_to_fac1		= $8C75		; C128 BASIC unsigned fixed to float (16-bit)
		_conv_fac1_to_i32 		= $8CC7		; C128 BASIC float to fixed (4-byte integer)
			;Xset_mantissa 			= $88FC		; C128 BASIC shift left mantissa (approximation for set_mantissa)
			;Xset_rest_fac1 		= $8917		; C128 BASIC trim FAC1 left (approximation for set_rest_fac1)
		_conv_ay_signed_to_fac1 = $793c		; givayf: convert to fac1 float
	.endweak
	
	conv_fac1_to_string .proc
		.c128_disable_rom
		lda #<$0100
		ldy #>$0100
		jsr basic._conv_fac1_to_string
		.c128_enable_rom
		rts
	.endproc

	conv_u8_to_fac1 .proc
		tay
		.c128_disable_rom
		jsr basic._conv_u8_to_fac1
		.c128_enable_rom
		rts
	.endproc

	conv_s8_to_fac1 .proc
		tay         ; y = low byte (original value)
		lda #$00    ; assume positive (high = $00)
		cpy #$80    ; compare low byte to $80 (check if negative)
		bcc pos     ; branch if positive (c clear for < $80)
		lda #$ff    ; negative: high = $ff
	pos:			; givayf: convert to fac1 float
		jsr basic._conv_ay_signed_to_fac1   
		rts
	.endproc

	conv_s16_to_fac1 .proc
		sta zpy
		sty zpa
		lda zpa
		ldy zpy
		jsr basic._conv_ay_signed_to_fac1   
		rts
	.endproc

	conv_u16_to_fac1 .proc
		sta zpy
		sty zpa
		lda zpa
		ldy zpy
		jsr $84E5	;	Set Up 16 Bit Fix-Float
		sec			;	setta il flag di carry, necessario per il trattamento unsigned
		jsr $8C75  	
					;   converte il valore fixed-point a 16 bit in floating point nel FAC1, 
					;	versione per unsigned; nota che $8C70 è la variante per signed.
		rts
	.endproc

	conv_fac1_to_DWord0 .proc
	jsr basic._conv_fac1_to_i32
	lda $67
	sta zpDWord0+0
	lda $66
	sta zpDWord0+1
	lda $65
	sta zpDWord0+2
	lda $64
	sta zpDWord0+3
	lda zpWord0+0
	ldy zpWord0+1
	rts
	.endproc

	; **********
	; *    LOAD
	; **********
	
	.weak
		_load5_fac1 = $8bd4	; Chiama MOVFM: carica 5 byte da (Y,A) a FAC1 ($61-$65)
		;X_load5_fac2 = $8AB4	; C128 BASIC unpack to FAC2
		_load5_fac2 = $8A89	; C128 BASIC unpack to FAC2 from current bank (ROMUPK)		
	.endweak

	load5_fac1	.proc
		jsr basic._load5_fac1
		rts
	.endproc
.comment
	Xload5_fac2	.proc
		jsr basic._load5_fac1
		jsr basic._store_fac1_in_fac2
		rts
	.endproc
.endc

	load5_fac2	.proc
		jsr basic._load5_fac2
		rts
	.endproc

	; **********
	; *    STORE
	; **********
	
	.weak
		_store5_fac1_xy 			 = $8C00	; C128 BASIC pack FAC1 to memory (X/Y addr)
		_store_fac1_in_fac2_round 	 = $8C47	; C128 BASIC round FAC1							TO CHECK
		_store_fac1_in_fac2 		 = $8C38	; C128 BASIC copy FAC1 to FAC2
		_store_fac2_in_fac1 	     = $8C28	; C128 BASIC copy FAC2 to FAC1
	.endweak
	
	store_fac2_in_fac1 .proc
		jmp basic._store_fac2_in_fac1
	.endproc
	

	; .................................................................... store_fac1
	store_fac1 .proc
		tax
		jmp basic._store5_fac1_xy
	.endproc
	
	store_fac1_in_fac2_round_round .proc
		jmp basic._store_fac1_in_fac2_round
	.endproc
	
	; **********
	; *    MATH
	; **********
	
	.weak
		_neg_fac1 			= $8FFA	; C128 BASIC negate FAC1
		_add_fac1_kb 		= $8848	; C128 BASIC add FAC1 to FAC2
		_mul_fac1_kb 		= $8A27	; C128 BASIC multiply FAC1 by FAC2
		_div_fac1_kb 		= $8B4C	; C128 BASIC divide FAC2 by FAC1
		_sub_fac1_kb 		= $8831	; C128 BASIC subtract FAC1 from FAC2
		_sgn_fac1_kb_a 		= $8C57	; C128 BASIC sign of FAC1 to A
		_sgn_fac1_kb_fac1	= $8C65	; C128 BASIC SGN to FAC1
	.endweak

	neg_fac1 .proc
		lda $63
		jmp basic._neg_fac1
	.endproc
	
	add_fac1 .proc
		lda $63
		jmp basic._add_fac1_kb
	.endproc
	
	sub_fac1 .proc
		lda $63
		jmp basic._sub_fac1_kb
	.endproc

	mul_fac1 .proc
		lda $63
		jmp basic._mul_fac1_kb
	.endproc

	div_fac1 .proc
		lda $63
		jmp basic._div_fac1_kb
	.endproc

	; ...................................................... true
	
	true .proc
		lda #1
		sec
		rts
	.endproc
	
	; ...................................................... false
	
	false .proc
		lda #0
		clc
		rts
	.endproc
	
	;	...................................................... not fac1

	zero	.byte  $00,$00,$00,$00,$00
	one	    .byte  $81,$00,$00,$00,$00

	not_fac1 .proc
		lda #<basic.zero
		ldy #>basic.zero
		jsr basic._fac1_compare  ; Compare FAC1 with memory at A/Y
		beq basic.true
		bne basic.false
	.endproc

	; ...................................................... sign -> a
	
	sgn_fac1_to_a .proc
		jmp basic._sgn_fac1_kb_a
	.endproc

	; ...................................................... sign -> fac1
	
	sgn_fac1_to_fac1 .proc
		jmp basic._sgn_fac1_kb_fac1
	.endproc

	; ...................................................... fac1_to_string
	
	fac1_to_string .proc
		.c128_disable_rom
		jsr basic.conv_fac1_to_string
		.c128_enable_rom
		lda #<$0100
		ldy #>$0100
		rts
	.endproc

	; **********
	; *    COMPARE
	; **********
	
	_fac1_compare 	  = $8C87 			; C128 BASIC compare FAC1 to memory
	_copy_fac1_to_mem = _store5_fac1_xy ; C128 BASIC pack FAC1 to memory

	
.endproc

;;;
;;
;

