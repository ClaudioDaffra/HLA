
;**********
;           BASIC
;**********

; libBASIC VIC20

basic	.proc

	; **********
	; 	PRINT
	; **********
	
	.weak	
		_print_unsigned_integer		= 	$ddcd 	; xa
		print_string				=   $cb1e   ; ay 	print a zero byte terminated petscii string		
	.endweak

	;	....................................................................	print_unsigned_integer
	
	print_unsigned_integer	.proc  				; ay 
		tax
		tya
		jmp basic._print_unsigned_integer
	.endproc

	;	....................................................................	print_unsigned_char

	print_unsigned_char	.proc  					; a
		tax
		lda #0
		jmp basic._print_unsigned_integer
	.endproc
	
	;	....................................................................	print_fac1

	print_fac1	.proc
		lda $ff
		pha
		jsr basic.conv_fac1_to_string
		pla
		sta $ff
		lda #<$0100
		ldy #>$0100
		jmp basic.print_string
	.endproc
	
	; **********
	; *	CONV
	; **********

	;	....................................................................	conv_fac1_to_string

	.weak

		conv_fac1_to_string		=	$dddd	; 	f1	convert fac1 to string at $100
		_conv_u8_to_fac1		=	$d3a2	;	a	Convert u8  held in Y to a FP number in FAC1
		conv_s8_to_fac1			=	$dc3c	;	a	Convert s8  held in A to a FP number in FAC1
		_conv_s16_to_fac1		=	$d391   ;   ay  Convert s16 held in Y/A (lo/high) to a FP number in FAC1

		;	There seems to be only a single routine in the ROMs, see Lothar English,
		;	“The Advanced Machine Language Book”, Abacus Software, 1984, p 28. ;
		;	IT is located at $BC9B and converts the floating-point value in the FAC ;
		;	(the fractional portion is truncated) into a whole number. ;
		; 	The four mantissa bytes ($62-$65) contain the value in a big-endian representation.

		conv_fac1_to_i32		=	$dc9b

		set_mantissa 			=  	$cf87
		set_rest_fac1			=  	$cf7e
	
	.endweak

	conv_u8_to_fac1	.proc
		tay
		jmp basic._conv_u8_to_fac1
	.endproc

	conv_u16_to_fac1	.proc
		sty zpy
		tay
		lda zpy
		sty $63
		sta $62
		ldx #$90
		sec
		jsr $DC49
		rts
	.endproc
	
	conv_s16_to_fac1	.proc
		sty zpy
		tay
		lda zpy
		jmp basic._conv_s16_to_fac1
	.endproc

	conv_fac1_to_DWord0	.proc
		jsr basic.conv_fac1_to_i32
		lda $65
		sta zpDWord0+0
		lda $64
		sta zpDWord0+1
		lda $63
		sta zpDWord0+2
		lda $62
		sta zpDWord0+3
		lda zpWord0+0
		ldy zpWord0+1
		rts
	.endproc
	
	; **********
	; *	LOAD
	; **********

	.weak

		load5_fac1					=	$dba2 	; ay	load fac1 from the 5 bytes pointed to by a/y (low)/(high).
												;		returns y = 0 and a loaded with the exponent, affecting the processor status flags.
		load5_fac2					=	$da8c 	; ay	load fac2 from the 5 bytes pointed to by a/y (low)/(high).
												;		returns with fac1's exponent in a.
	.endweak

	; **********
	; *	STORE
	; **********

	.weak
		store5_fac1_xy				=	$dbd4 	; xy	stores fac1 as 5 bytes at the address pointed to by x/y (low)/(high).
												;		checks the rounding byte ($70) beforehand, and rounds if the msb is set.
												;		as with loading, it returns with the exponent in a.
		store_fac1_in_fac2_round	=	$dc0c 	; f1,f2	stores fac1 in fac2, rounding if necessary.
		store_fac1_in_fac2			=	$dc0f 	; f1,f2	stores fac1 in fac2  skipping rounnding
		store_fac2_in_fac1			=	$dbfc 	; f1,f2	stores fac2 in fac1

	.endweak

	;	....................................................................	store_fac1

	store_fac1	.proc							; ay
		tax
		jmp basic.store5_fac1_xy
	.endproc

	store_fac2_inc_fac1	=	$dbfc ; Stores FAC2 in FAC1.


	; **********
	; *	MATH
	; **********

	.weak
	
		;	kLocalAsmReal03			.byte  $8b,$00,$18,$93,$74 	;  //  1024.768	bit segno ( ?000:0000 )
		;	kLocalAsmReal04			.byte  $8b,$80,$18,$93,$74 	;  // -1024.768	bit segno ( 1000:0000 ) $80
			
		neg_fac1   					= 	$dfb4	;	49076	makes fac1 negative
		neg_fac1_kb					= 	$dfb4	;	49076	makes fac1 negative

		add_fac1_kb					=	$d86a   ;	Entry if FAC2 already loaded.
												;	The accumulator must load FAC1 exponent ($61) immediately
												;	before calling to properly set the zero flag.
		mul_fac1_kb					=	$da2b 	; 	Entry if FAC2 already loaded.
												;	Accumulator must load FAC1 exponent ($61) beforehand to set the zero

		div_fac1_kb 				=   $db12   ;   fac1    :=  fac1    /   fac2    ,   remainer fac2

		sub_fac1_kb					=	$d853	; 	Entry if FAC2 already loaded.

		sgn_fac1_kb_a				=	$dc2b 	;	Evaluate sign of FAC1 (returned in A)

		sgn_fac1_kb_fac1			=	$dc39	;	Evaluate sign of FAC1 (returned in FAC1)

	.endweak
	
	add_fac1	.proc
	
		lda $61
		jmp basic.add_fac1_kb
	
	.endproc
	
	sub_fac1	.proc
	
		lda $61
		jmp basic.sub_fac1_kb
	
	.endproc

	mul_fac1	.proc
	
		lda $61
		jmp basic.mul_fac1_kb
	
	.endproc

	div_fac1	.proc
	
		lda $61
		jmp basic.div_fac1_kb
	
	.endproc
	
	;	...................................................... true

	true	.proc
		lda #1
		rts
	.endproc

	;	...................................................... false

	false	.proc
		lda #0
		rts
	.endproc

	;	...................................................... not fac1

	zero	.byte  $00,$00,$00,$00,$00
	one	    .byte  $81,$00,$00,$00,$00
	
	not_fac1	.proc
		lda #<basic.zero
		ldy #>basic.zero
	;start	
		jsr $dc5b  ; -> a ;;;; compare float accu to float indexed by XY
		; = Compare FAC1 with memory contents at A/Y (lo/high)
		;	$00 FAC1 == AY
		;	$01 FAC1  > AY
		;   $FF FAC1  < AY
		beq basic.true
		bne basic.false
		;rts
	.endproc

	;	...................................................... sign -> a
	
	sgn_fac1_to_a	.proc
			jmp basic.sgn_fac1_kb_a
	.endproc
	
	;	...................................................... sign -> fac1

	sgn_fac1_to_fac1	.proc
			; 1 	+
			; 255	-
			; 0		0
			jmp basic.sgn_fac1_kb_fac1
	.endproc

	fac1_to_string .proc
		jsr basic.conv_fac1_to_string
		lda #<$0100
		ldy #>$0100
		rts
	.endproc
	
	; **********
	; *	COMPARE
	; **********

	; $dc5b = Compare FAC1 with memory contents at A/Y (lo/high)
	; 
	; Returns with A = 
	;     $00 if the values are equal, 
	;     $01 if FAC1 > MEM, or 
	;     $ff if FAC1 < MEM. Uses the vector at $24 to address the variable, 
	;     leaving FAC1 and FAC2 unchanged. 

	fac1_compare 		=   $dc5b 

	copy_fac1_to_mem	=   $dbd4   ;   input   -> xy
	
.endproc

;;;
;;
;

