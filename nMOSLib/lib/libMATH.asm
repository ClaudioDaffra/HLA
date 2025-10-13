
;**********
;           MATH
;**********

math	.proc

	zero	.byte  $00,$00,$00,$00,$00	

	;	...................................................... swap zpWord0 <-> ay

	swap_zpWord0_ay	.proc			;	fast
		ldx zpWord0+0
		sta zpWord0+0
		tya
		ldy zpWord0+1
		sta zpWord0+1
		txa
		rts
	.endproc
	
	;	...................................................... neg f40	real

	neg_f40	.proc

		jsr basic.neg_fac1
		rts

	.endproc
	
	;	...................................................... true

	true	.proc
		lda #1
		ldy #0
		sec
		rts
	.endproc

	;	...................................................... false

	false	.proc
		lda #0
		tay
		clc
		rts
	.endproc

	;	...................................................... not u8,s8	byte

	not_u8 = not_u8s8
	not_s8 = not_u8s8
	
	not_u8s8	.proc
		cmp #0
		beq math.true		;	se è ==0  -> 1
		bne math.false		;	se è !=0  -> 0
	.endproc

	;	...................................................... not u16,s16	word

	not_u16 = not_u16s16
	not_s16 = not_u16s16
	
	not_u16s16	.proc
		cmp #0
		beq +
		bne math.false	;	se a == 0 	-> 1
	+
		cpy #0
		beq math.true	;	se ay == 0 -> 1
		bne math.false	;	else 	   -> 0
	.endproc

	;	...................................................... not f40		real

	not_f40	.proc

		jmp basic.not_fac1

	.endproc

	;	...................................................... add u16,s16	word		[ok]
	;
	;	ay := ay + zpWord0

	add_u16 = add_u16s16
	add_s16 = add_u16s16
	
	add_u16s16	.proc

		sta	zpWord1+0
		sty zpWord1+1
		clc
		lda zpWord0+0
		adc zpWord1+0
		tax
		lda zpWord0+1
		adc zpWord1+1
		tay
		txa

		rts

	.endproc

	;	...................................................... add_f40

	;	dest	fac1 := fac1 + fac2

	add_f40	.proc

		lda $61
		jsr basic.add_fac1_kb
		rts

	.endproc

	;	...................................................... mul_f40
	;
	;	dest	fac1 := fac1 * fac2

	mul_f40	.proc

		lda $61
		jsr basic.mul_fac1_kb
		rts

	.endproc

	; --------------------------------------------------------------- mul_bytes_into_u16
	;
	;   multiply 2 bytes A and Y,  as word in A/Y (unsigned)
	;
	;   input   :   a,y
	;   output  :   a*y
	;

	mul_u8    .proc
			;ldy zpWord0+0
	;no_compiler		
		sta  zpa
		sty  zpy
		stx  zpx

		lda  #0
		ldx  #8
		lsr  zpa
	-
		bcc  +
		clc
		adc  zpy
	+
		ror  a
		ror  zpa
		dex
		bne  -
		tay
		lda  zpa
		ldx  zpx

		rts
	.endproc

	mul_s8   .proc 
		;ldy zpWord0+0
	;no_compiler	
		sta t1
		sty t2                               
		jsr math.mul_u8                           
		; ay
		sta zpWord0+0
		sty zpWord0+1

		; apply sign (see c=hacking16 for details).
		
		lda t1                                     
		bpl j1                                     
		sec                                    
		lda zpWord0+1                          
		sbc t2                                 
		sta zpWord0+1                          
	j1                                          
		lda t2                                     
		bpl j2                                     
		sec                                    
		lda zpWord0+1                          
		sbc t1                                 
		sta zpWord0+1                          
	j2                                          

		lda zpWord0+0
		ldy zpWord0+1
	rts

	t1		.byte 0
	t2		.byte 0

	.endproc 

	;..........................................................................
	;
	;   multiply two 16-bit words into a 32-bit zpDWord1  (signed and unsigned)
	;
	;      input    :
	;
	;           A/Y             = first  16-bit number,
	;           zpWord0 in ZP   = second 16-bit number
	;
	;      output   :
	;
	;           zpDWord1  4-bytes/32-bits product, LSB order (low-to-high)
	;			a/y		  16 bit
	;
	;     result    :     zpDWord1  :=  zpWord0 * zpWord1
	;
	;   LSB 0123
	;
	;   0   zpWord1+0   low
	;   1   zpWord1+1
	;   2   zpWord1+2   high
	;   3   zpWord1+3

	mul_u16	= mul_u16s16
	mul_s16	= mul_u16s16
	
	mul_u16s16    .proc

	result = zpDWord1

			sta  zpWord1
			sty  zpWord1+1
			stx  zpx
	mult16
			lda  #0
			sta  zpDWord1+2     ; clear upper bits of product
			sta  zpDWord1+3
			ldx  #16            ; for all 16 bits...
	-
			lsr  zpWord0+1      ; divide multiplier by 2
			ror  zpWord0
			bcc  +
			lda  zpDWord1+2     ; get upper half of product and add multiplicand
			clc
			adc  zpWord1
			sta  zpDWord1+2
			lda  zpDWord1+3
			adc  zpWord1+1
	+
			ror  a              ; rotate partial product
			sta  zpDWord1+3
			ror  zpDWord1+2
			ror  zpDWord1+1
			ror  zpDWord1
			dex
			bne  -
			ldx  zpx

			lda zpDWord1+0
			ldy zpDWord1+1

			rts

	.endproc

	; ...................................................... mul_u8s8_fast
	;
	;	https://www.nesdev.org/wiki/8-bit_Multiply
	;
	;	input	:	A moltiplicando
	;				Y moltiplicatore
	; 	output	:	A/Y
	;

	mul_u8_fast .proc

			lsr
			sta zpa
			tya
			beq mul8_early_return
			dey
			sty zpy
			lda #0

			;0
			bcc +
			adc zpy
		+
			ror
			;1
			ror zpa
			bcc +
			adc zpy
		+
			ror
			;2
			ror zpa
			bcc +
			adc zpy
		+
			ror
			;3
			ror zpa
			bcc +
			adc zpy
		+
			ror
			;4
			ror zpa
			bcc +
			adc zpy
		+
			ror
			;5
			ror zpa
			bcc +
			adc zpy
		+
			ror
			;6
			ror zpa
			bcc +
			adc zpy
		+
			ror
			;7
			ror zpa
			bcc +
			adc zpy
		+
			ror

			tay
			lda zpa
			ror

		mul8_early_return

			rts

	.endproc
	
	mul_s8_fast   .proc 

		sta t1
		sty t2                               
		jsr math.mul_u8_fast                          
		; ay
		sta zpWord0+0
		sty zpWord0+1

		; apply sign (see c=hacking16 for details).
		
		lda t1                                     
		bpl j1                                     
		sec                                    
		lda zpWord0+1                          
		sbc t2                                 
		sta zpWord0+1                          
	j1                                          
		lda t2                                     
		bpl j2                                     
		sec                                    
		lda zpWord0+1                          
		sbc t1                                 
		sta zpWord0+1                          
	j2                                          

		lda zpWord0+0
		ldy zpWord0+1
	rts

	t1		.byte 0
	t2		.byte 0

	.endproc 

	; ...................................................... mul_u16s16_fast
	;
	; https://github.com/TobyLobster/multiply_test/blob/main/tests/mult55.a
	; mult55.a
	; from The Merlin 128 Macro Assembler disk, via 'The Fridge': 
	; http://www.ffd2.com/fridge/math/mult-div.s
	; TobyLobster: with an optimisation for speed (changing pha/pla to tax/txa), 
	;  then fully unrolled
	; (see mult2)
	;
	; 16 bit x 16 bit unsigned multiply, 32 bit result
	; Average cycles: 483.50
	; 344 bytes
	; acc*aux -> [acc,acc+1,ext,ext+1] (low,hi) 32 bit result

	mul_u16_fast = mul_u16s16_fast
	mul_s16_fast = mul_u16s16_fast
	
	mul_u16s16_fast	.proc

	aux = zpWord0       ; zpWord0	2 bytes   input1
	acc = zpWord1       ; AY		2 bytes   input2   } result
	ext = zpWord2       ; zpWord	2 bytes            }

		; (acc, acc+1, ext, ext+1) = (aux, aux+1) * (acc, acc+1)

		sta zpWord1
		sty zpWord1+1

	mult

		lda #0                          ; A holds the low byte of ext (zero for now)
		sta ext+1                       ; high byte of ext = 0
		lsr acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 1
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 2
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 3
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 4
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+

		; loop step 5
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 6
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 7
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 8
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 9
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 10
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 11
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 12
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 13
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 14
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 15
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		; loop step 16
		ror ext+1                       ; }
		ror                             ; }
		ror acc+1                       ; } acc_ext >> 1
		ror acc                         ; }
		bcc +                           ; skip if carry clear

		clc                             ;               }
		adc aux                         ;               }
		tax                             ; remember A    }
		lda aux+1                       ;               } ext += aux
		adc ext+1                       ;               }
		sta ext+1                       ;               }
		txa                             ; recall A
	+
		sta ext

		lda acc
		ldy acc+1

		rts

	.endproc
.comment	
	;........................................ div_s8
	;
	;   divide A by Y, result quotient in A, remainder in Y   (signed)
	;
	;   Inputs:
	;       a       =   8-bit numerator
	;       y       =   8-bit denominator
	;   Outputs:
	;       a       =   a / y       ( signed   )
	;       y       =   remainder   ( unsigned )
	;

	; TODO TO CHECK ERROR ?!
	
	div_s8    .proc
			;ldy zpWord0+0			; compiler
	;no_compiler
			sta  zpa
			tya
			eor  zpa
			php             ; save sign
			lda  zpa
			bpl  +
			eor  #$ff
			sec
			adc  #0         ; make it positive
	+
			pha
			tya
			bpl  +
			eor  #$ff
			sec
			adc  #0         ; make it positive
			tay
	+
			pla
			jsr  internal_div_u8
			sta  zpByte0
			plp
			bpl  +
			tya
			eor  #$ff
			sec
			adc  #0         ; negate result
							;   a   result
			ldy zpByte0     ;   y   remainder
	+
			rts

	.endproc
.endc

	; ---------------------------------------------------- lib
	; s8_division - Signed 8-bit division A / Y, result in A
	; Uses zero page: zpa, zpx, zpy, zpWord0, zpWord0+1, tmp
	; Assumes divisor != 0, no overflow check (result fits in s8)
	; Clobbers X register

	div_s8 .proc
		sta zpa         ; store dividend
		sty zpy         ; store divisor

		; Compute result sign (bit 7): sign(A) XOR sign(Y)
		lda zpa
		and #$80
		sta zpWord0+1   ; temp sign A
		lda zpy
		and #$80
		eor zpWord0+1
		sta zpWord0+1   ; sign result

		; Absolute value of dividend
		lda zpa
		bpl pos_divid
		eor #$FF
		clc
		adc #1
	pos_divid
		sta zpWord0     ; unsigned dividend

		; Absolute value of divisor
		lda zpy
		bpl pos_divis
		eor #$FF
		clc
		adc #1
	pos_divis
		sta tmp         ; unsigned divisor
		; Skip div by zero handling

		; Unsigned division setup
		lda #0
		sta zpa         ; quotient = 0
		sta zpx         ; remainder = 0
		ldx #8          ; bit counter

	div_loop
		asl zpWord0     ; shift dividend bit into carry
		rol zpx         ; shift into remainder
		lda zpx
		cmp tmp
		bcc no_sub
		sbc tmp
		sta zpx
		sec
		jmp set_bit
	no_sub
		clc
	set_bit
		rol zpa         ; shift bit (C) into quotient
		dex
		bne div_loop

		; Apply sign to quotient
		lda zpWord0+1   ; sign result
		beq pos_result
		lda zpa
		eor #$FF
		clc
		adc #1
		sta zpa
	pos_result

		lda zpa         ; load result into A
		rts
	.endproc


	internal_div_u8    .proc

			sty  zpy
			sta  zpa
			stx  zpx

			lda  #0
			ldx  #8
			asl  zpa
	-
			rol  a
			cmp  zpy
			bcc  +
			sbc  zpy
	+
			rol  zpa
			dex
			bne  -
			ldy  zpa
			ldx  zpx

			rts
	.endproc

	;........................................ div_u8
	;
	;   divide A by Y, result quotient in A, remainder in Y   (unsigned)
	;
	;   Inputs:
	;       a       =   8-bit numerator
	;       y       =   8-bit denominator
	;   Outputs:
	;       a       =   a / y       ( unsigned  )
	;       y       =   remainder   ( unsigned  )
	;
	;	compiler	=   y/a
	
	div_u8    .proc
		;ldy zpWord0+0			; compiler
	;no_compiler		
		sta zpByte0
		sty zpByte1

		lda #0
		ldx #8
		asl zpByte0
	L1
		rol
		cmp zpByte1
		bcc L2
		sbc zpByte1
	L2
		rol zpByte0
		dex
		bne L1
		tay
		lda zpByte0

		rts
	.endproc

	;.......................................................................    div_s16
	;
	;   divide two unsigned words (16 bit each) into 16 bit results
	;
	;    input:
	;            zpWord0    :   16 bit number,
	;            A/Y        :   16 bit divisor
	;    output:
	;            zpWord1    :   16 bit remainder,
	;            A/Y        :   16 bit division result
	;            zpWord0
	;            flag V     :   1 = division by zero
	;
	;   signed word division: make everything positive and fix sign afterwards

	div_s16    .proc
.comment
		cmp     #$00
		bne     check_divByZero
		cpy     #$00
		bne     check_divByZero
		sev
		rts

	 check_divByZero
.endc
		sta saveA
		sty saveY

		sta  zpWord1
		sty  zpWord1+1
		lda  zpWord0+1
		eor  zpWord1+1
		php            ; save sign
		lda  zpWord0+1
		bpl  +
		lda  #0
		sec
		sbc  zpWord0
		sta  zpWord0
		lda  #0
		sbc  zpWord0+1
		sta  zpWord0+1
	+
		lda  zpWord1+1
		bpl  +
		lda  #0
		sec
		sbc  zpWord1
		sta  zpWord1
		lda  #0
		sbc  zpWord1+1
		sta  zpWord1+1
	+
		tay
		lda  zpWord1
		jsr  div_u16
		;
		pha
		lda zpWord1
		sta zpWord3
		lda zpWord1+1
		sta zpWord3+1
		pla
		;
		plp            ; restore sign
		bpl  +
		sta  zpWord1
		sty  zpWord1+1
		lda  #0
		sec
		sbc  zpWord1
		pha
		lda  #0
		sbc  zpWord1+1
		tay
		pla
	+
		pha
		lda zpWord3
		sta zpWord1
		lda zpWord3+1
		sta zpWord1+1
		pla

		pha
		lda saveA
		sta zpWord3
		lda saveY
		sta zpWord3+1
		pla

		sta zpWord0
		sty zpWord0+1

		clv
		rts

	 saveA  .byte   0
	 saveY  .byte   0

	.endproc

	;.......................................................................    div_u16		[ok]
	;
	;   divide two unsigned words (16 bit each) into 16 bit results
	;
	;    input:
	;            zpWord0    :   16 bit number,
	;            A/Y        :   16 bit divisor
	;    output:
	;            zpWord1    :   in ZP: 16 bit remainder,
	;            A/Y        :   16 bit division result
	;            zpWord0
	;            flag V     :   1 = division by zero
	;
	;	ay/x	:= zpWord0  /	ay
	
	div_u16    .proc

	dividend    = zpWord0
	remainder   = zpWord1
	result      = zpWord0   ;   dividend
	divisor     = zpWord3

		jsr swap_zpWord0_ay 
.comment		
		cmp     #$00
		bne     check_divByZero
		cpy     #$00
		bne     check_divByZero
		sev
		rts
	 check_divByZero
.endc
		sta  divisor
		sty  divisor+1
		stx  zpx
		lda  #0             ;preset remainder to 0
		sta  remainder
		sta  remainder+1
		ldx  #16            ;repeat for each bit: ...
	-
		asl  dividend       ;dividend lb & hb*2, msb -> Carry
		rol  dividend+1
		rol  remainder      ;remainder lb & hb * 2 + msb from carry
		rol  remainder+1
		lda  remainder
		sec
		sbc  divisor        ;substract divisor to see if it fits in
		tay                 ;lb result -> Y, for we may need it later
		lda  remainder+1
		sbc  divisor+1
		bcc  +              ;if carry=0 then divisor didn't fit in yet

		sta  remainder+1    ;else save substraction result as new remainder,
		sty  remainder
		inc  result         ;and INCrement result cause divisor fit in 1 times
	+
		dex
		bne  -

		lda  result
		ldy  result+1
		ldx  zpx

		clv
		rts

	.endproc

	;	...................................................... swap_fac1_fac2 COMMODORE 64
	
	swap_fac1_fac2	.proc
	
		lda $61
		ldx $69
		sta $69
		stx $61

		lda $62
		ldx $6a
		sta $6a
		stx $62	
		
		lda $63
		ldx $6b
		sta $6b
		stx $63		

		lda $64
		ldx $6c
		sta $6c
		stx $64	

		lda $65
		ldx $6d
		sta $6d
		stx $65	

		lda $66
		ldx $6e
		sta $6e
		stx $66	

		lda $67		;	?
		ldx $6f
		sta $6f
		stx $67	

		lda $68		;	?
		ldx $70
		sta $70
		stx $68	

		rts
		
	.endproc
	
	;	...................................................... div_f40
	;
	;	dest	fac1 := fac1 * fac2

	div_f40	.proc

	fac1_div_by_fac2
	
		jsr math.swap_fac1_fac2

	fac2_div_by_fac1

		lda $61
		jsr basic.div_fac1_kb

		rts

	.endproc

	;   ........................................................ mod %

	mod_u8    .proc
		jsr  math.div_u8
		tya                 ;remainer
		rts
	.pend

	mod_u16    .proc
		jsr math.div_u16
		lda zpWord1
		ldy zpWord1+1   ;   remainder
		rts
	.pend

	mod_s8    .proc
		jsr  math.div_s8
		tya                 ;remainer
		rts
	.pend

	mod_s16    .proc
		jsr  math.div_s16
		lda zpWord1
		ldy zpWord1+1   ;   remainder
		rts
	.pend

	;	......................................................  
	;
	;	dest	fac1 % fac2 := fac1 / fac2 <- mod 
	;

	; Define temporary storage areas (5 bytes each for floating point values)
	bufferFac1 = $02A7
	bufferFac2 = $02AC
	; Define BASIC ROM routine addresses (c64/vic20)
.comment	
	basic_store_fac1 				= $B/D-BD4		; FACMEM: store FAC1 to mem, X=low Y=high
	basic_copy_fac2_to_fac1 		= $B/D-BBFC		; ARGFAC: copy FAC2 to FAC1
	basic_load5_fac1 				= $B/D-BBA2		; MEMFAC: load to FAC1, A=low Y=high
	basic_load5_fac2 				= $B/D-BA8C		; MEMARG: load to FAC2, A=low Y=high
	basicROM_fdivt 					= $B/D-BB12		; FDIVT: FAC1 = FAC2 / FAC1
	basicROM_int 					= $B/D-BCCC		; INT: FAC1 = INT(FAC1)
	basicROM_fmultt 				= $B/D-BA2B		; FMULTT: FAC1 = FAC1 * FAC2
	basicROM_fsubt 					= $B/D-B853		; FSUBT: FAC1 = FAC2 - FAC1
.endc	

	; fac1::1024.768 %% fac2::1.2 		:= fac1::1.168
	
	mod_f40	.proc
		; Compute FAC1 = FAC1 % FAC2 using formula: FAC1 - FAC2 * INT(FAC1 / FAC2)
		; Save original FAC1 (A) to bufferFac1
		ldx #<bufferFac1
		ldy #>bufferFac1
		jsr basic_store_fac1
		; Save original FAC2 (B) to bufferFac2
		jsr basic_copy_fac2_to_fac1     ; Copy FAC2 to FAC1
		ldx #<bufferFac2
		ldy #>bufferFac2
		jsr basic_store_fac1     ; Store (now B in FAC1) to bufferFac2
		; Restore original FAC1 (A) from bufferFac1
		lda #<bufferFac1
		ldy #>bufferFac1
		jsr basic_load5_fac1
		; Now compute A / B
		; Load B to FAC1
		lda #<bufferFac2
		ldy #>bufferFac2
		jsr basic_load5_fac1
		; Load A to FAC2
		lda #<bufferFac1
		ldy #>bufferFac1
		jsr basic_load5_fac2
		; Divide: FAC1 = FAC2 / FAC1 = A / B
		jsr basicROM_fdivt
		; Apply INT: FAC1 = INT(A / B)
		jsr basicROM_int
		; Load B to FAC2
		lda #<bufferFac2
		ldy #>bufferFac2
		jsr basic_load5_fac2
		; Multiply: FAC1 = FAC1 * FAC2 = INT(A / B) * B
		jsr basicROM_fmultt
		; Load A to FAC2
		lda #<bufferFac1
		ldy #>bufferFac1
		jsr basic_load5_fac2
		; Subtract: FAC1 = FAC2 - FAC1 = A - (INT(A / B) * B)
		jsr basicROM_fsubt
		rts
	.endproc

	;	...................................................... sub_u16s16		[OK]
	;
	;	ay := ay - zpWord0

	sub_u16	=	sub_u16s16
	sub_s16 = 	sub_u16s16
	sub_u16s16	.proc

		sta	zpWord1+0
		sty zpWord1+1

	start

		sec
		lda zpWord1
		sbc zpWord0
		tax
		lda zpWord1+1
		sbc zpWord0+1
		tay
		txa

		rts

	.endproc

	;	...................................................... sub_f40

	;	dest	fac1 := fac1 - fac2

	sub_f40	.proc
		jsr math.swap_fac1_fac2
		lda $61
		jsr basic.sub_fac1_kb
		rts
	.endproc

	;	...................................................... neg u16 s16		[ok]

	neg_u16	=  neg_u16s16
	neg_s16	=  neg_u16s16
	
	neg_u16s16 .proc

		sta zpWord0
		sty zpWord0+1

		lda zpWord0
		eor #$ff
		sta zpWord1

		lda zpWord0+1
		eor #$ff
		sta zpWord1+1

		clc
		lda zpWord1
		adc #1
		sta zpWord1

		lda zpWord1+1
		adc #0
		sta zpWord1+1
		
		lda zpWord1+0
		ldy zpWord1+1

		rts
			
	.endproc

	;	...................................................... real to string

	f40_to_string .proc
		jmp basic.conv_fac1_to_string
	.endproc
	
	;

	neg_u8 .proc
		eor #$ff
		clc
		adc #$01
		rts
	.endproc

	neg_s8	.proc
		eor #$ff
		clc
		adc #$01
		rts
	.endproc

	; .......................... mul word by 1
	; ay := ay * 1(zpWord0)
	mul_word_by_1	.proc

		rts
	.endproc
	
	; .......................... mul word by 2
	; ay := ay * 2(zpWord0)
	mul_word_by_2	.proc

		clc
		asl			; shift a sinistra il low, bit7 -> c
		tax
		tya         ; a = high
		rol			; ruota a sinistra con carry: entra il carry dal low
		tay         ; y = nuovo high
		txa
		rts
	.endproc

	mul_word_by_5	.proc
		sty  zpWord0+1
		sta  zpWord1
		sty  zpWord1+1
		asl
		rol  zpWord0+1
		asl
		rol  zpWord0+1
		clc
		adc  zpWord1
		tax
		lda  zpWord0+1
		adc  zpWord1+1
		tay
		txa
		rts
	.endproc

	; ---------------------------------------------------------
	; multiplies a 16-bit number (low byte in a, high byte in y) by an 8-bit number in x.
	; returns the low 16 bits of the result in a (low) and y (high).
	; assumes the result fits in 16 bits or truncates if overflow.
	; ay := ay*x
	;
	mul_word_by_x .proc
		sta zpWord0+0     	; store low byte of num
		sty zpWord0+1     	; store high byte of num
		stx zpx     		; store multiplier
		lda #0
		sta zpa     		; result low
		sta zpy     		; result mid (high of 16-bit)
		sta tmp     		; result high (overflow)
		ldx #8      		; loop counter for 8 bits
	loop
		lsr zpx     		; shift multiplier right, bit to carry
		bcc noadd   		; if bit 0, skip add
		clc         		; clear carry for addition
		lda zpa
		adc zpWord0+0
		sta zpa
		lda zpy
		adc zpWord0+1
		sta zpy
		lda tmp
		adc #0
		sta tmp
	noadd
		asl zpWord0+0     ; shift num left
		rol zpWord0+1
		dex         	; decrement counter
		bne loop    	; repeat until 8 bits done
		lda zpa     	; load low byte of result into a
		ldy zpy     	; load high byte of result into y
		rts         	; return (ignores overflow in tmp if any)
	.endproc

	;	-------------------------------------- COMPARE		< <= > >= ?=

	.comment
	
		byte 
		
			;	-------------------------------------- u8	?=
			s8_cmp_eq	=	u8s8_cmp_eq
			u8_cmp_eq	=	u8s8_cmp_eq
			u8s8_cmp_eq	.proc		;	?=
			
			u8_cmp_lt   .proc		;	<
			s8_cmp_lt   .proc		;	<
			u8_cmp_le   .proc		;	<=
			s8_cmp_le   .proc		;	<=
			
			u8_cmp_gt   .proc		;	>
			s8_cmp_gt   .proc		;	>
			u8_cmp_ge   .proc		;	>=
			s8_cmp_ge   .proc		;	>=
		
		word

			s16_cmp_eq = u16s16_cmp_eq
			u16_cmp_eq = u16s16_cmp_eq
			u16s16_cmp_eq .proc		;	?=
			
			u16_cmp_lt	.proc		;	<
			u16_cmp_le	.proc		;	<=
			s16_cmp_lt	.proc		;	<
			s16_cmp_le	.proc		;	<=

			s16_cmp_ge	.proc		;	>=
			s16_cmp_gt  .proc		;	>
			u16_cmp_ge  .proc		;	>=
			u16_cmp_gt  .proc		;	>
		
		real 

			f40_cmp_lt	.proc		;	<
			f40_cmp_le	.proc		;	<=
			f40_cmp_gt	.proc		;	>
			f40_cmp_ge	.proc		;	>=
			f40_cmp_eq	.proc		;	?=
			f40_cmp_ne	.proc		;	!=
			
	.endc
	
	;	-------------------------------------- u8 a ?= zpWord0+0
	
	s8_cmp_eq	=	u8s8_cmp_eq
    u8_cmp_eq	=	u8s8_cmp_eq
	u8s8_cmp_eq	.proc
        cmp  zpWord0+0
        bne  +
        jmp math.true
    +
        jmp math.false
	.endproc

	;	-------------------------------------- u8 a != zpWord0+0
	
	s8_cmp_ne	=	u8s8_cmp_ne
    u8_cmp_ne	=	u8s8_cmp_ne
	u8s8_cmp_ne	.proc
        cmp  zpWord0+0
        bne  +
        jmp math.false
    +
        jmp math.true
	.endproc
	
	;	-------------------------------------- u8 a < zpWord0+0

    u8_cmp_lt   .proc
        cmp  zpWord0+0
        bcs  +
        jmp math.true
    +
        jmp math.false
    .pend

	;	-------------------------------------- s8  a < zpWord0+0

    s8_cmp_lt   .proc
        sec
        sbc  zpWord0+0
        bvc  +
        eor  #$80
    +               
        bpl  +
        jmp math.true
    +
        jmp math.false
    .pend

	;	-------------------------------------- u8  a <= zpWord0+0

    u8_cmp_le   .proc
        cmp  zpWord0+0
        beq  +
        jmp math.u8_cmp_lt
	+
		jmp math.true
    .pend
	
	;	-------------------------------------- s8  a <= zpWord0+0

    s8_cmp_le   .proc
        cmp  zpWord0+0
        beq  +
        jmp math.s8_cmp_lt
	+
		jmp math.true
    .pend

	;	-------------------------------------- u8 >
	
    u8_cmp_gt   .proc
        cmp  zpWord0
        bcc  +
        beq  +
		jmp math.true
    +
		jmp math.false
    .pend

	;	-------------------------------------- s8 >
	
    s8_cmp_gt   .proc
        clc
        sbc  zpWord0
        bvc  +
        eor  #$80
    +               
        bpl  +
        bmi  false
    +
		jmp math.true
    false
		jmp math.false
        
    .pend  
	
	;	-------------------------------------- u8  a >= zpWord0+0

    u8_cmp_ge   .proc
        cmp  zpWord0+0
        beq  +
        jmp math.u8_cmp_gt
	+
		jmp math.true
    .pend
	
	;	-------------------------------------- s8  a >= zpWord0+0
	
    s8_cmp_ge   .proc
        cmp  zpWord0+0
        beq  +
        jmp math.s8_cmp_gt
	+
		jmp math.true
    .pend

	;	-------------------------------------- u16 ?=
	;	-------------------------------------- s16 ?=
	
	;   input ay zpWord0+0/1 , output a
	
	s16_cmp_eq = u16s16_cmp_eq
	u16_cmp_eq = u16s16_cmp_eq

	u16s16_cmp_eq .proc
		sta  zpWord1+0
		sty  zpWord1+1
		lda  zpWord0
		cmp  zpWord1
		bne  +
		ldy  zpWord0+1
		cpy  zpWord1+1
		bne  +
		jmp math.true
	+
		jmp math.false
	.endproc	

	s16_cmp_ne = u16s16_cmp_ne
	u16_cmp_ne = u16s16_cmp_ne

	u16s16_cmp_ne .proc
		sta  zpWord1+0
		sty  zpWord1+1
		lda  zpWord0
		cmp  zpWord1
		bne  +
		ldy  zpWord0+1
		cpy  zpWord1+1
		bne  +
		jmp math.false
	+
		jmp math.true
	.endproc	
	
	;	-------------------------------------- u16 <
    
    u16_cmp_lt	.proc
		cpy zpWord0+1
		bcc vero
		bne falso
		cmp zpWord0
		bcc vero
		jmp math.false
	vero
		jmp math.true
	falso
		jmp math.false
	.endproc	

	;	-------------------------------------- u16 <=
	
    u16_cmp_le	.proc
		cpy zpWord0+1
		bcc vero
		bne falso
		cmp zpWord0
		bcc vero
		beq vero
		jmp math.false
	vero
		jmp math.true
	falso
		jmp math.false
	.endproc	

	;	-------------------------------------- s16 <
	
    s16_cmp_lt	.proc

		sta  zpWord1+0
		sty  zpWord1+1
        ldy  zpWord0+1
        lda  zpWord0
        cmp  zpWord1
        tya
        sbc  zpWord1+1
        bvc  +
        eor  #$80
    +               
        bpl  s16_cmp_lt_else
		jmp math.true
    s16_cmp_lt_else
		jmp math.false
	.endproc	

	;	-------------------------------------- s16 <=
	
    s16_cmp_le	.proc
    
		tya
		eor zpWord0+1
		and #$80
		beq signs_same
		tya
		and #$80
		beq falso
		jmp vero
	signs_same
		cpy zpWord0+1
		bcc vero
		bne falso
		cmp zpWord0
		bcc vero
		beq vero
		jmp math.false
	vero
		jmp math.true
	falso
		jmp math.false
	.endproc	

	;	-------------------------------------- s16 >=

	s16_cmp_ge	.proc
		pha              ; save lhs low byte
		tya              ; a = lhs high
		eor zpWord0+1    ; xor with rhs high to check signs
		bpl same_signs   ; branch if signs same (result bit 7 = 0)
		; signs different
		tya              ; a = lhs high
		bmi lhs_neg      ; if lhs negative (bit 7 = 1), then lhs < rhs
		pla              ; discard saved low byte
		jmp math.true    ; lhs positive, rhs negative: true
	lhs_neg:
		pla              ; discard saved low byte
		jmp math.false   ; lhs negative, rhs positive: false
	same_signs:
		tya              ; a = lhs high
		cmp zpWord0+1    ; compare with rhs high (unsigned)
		bcc lhs_smaller  ; lhs high < rhs high: false
		beq compare_lows ; equal: compare lows
		pla              ; discard saved low byte (highs unequal, lhs > rhs)
		jmp math.true    ; true
	lhs_smaller:
		pla              ; discard saved low byte
		jmp math.false   ; false
	compare_lows:
		pla              ; restore a = lhs low
		cmp zpWord0      ; compare with rhs low (unsigned)
		bcs ge_true      ; >= : true
		jmp math.false   ; < : false
	ge_true:
		jmp math.true    ; true

	.endproc

	;	-------------------------------------- s16 >

	s16_cmp_gt .proc
		cmp zpWord0+1
		bne check_high_byte
		tya
		cmp zpWord0
		beq falso
		bcs vero
		jmp falso
	check_high_byte
		bcs check_both_positive
		bit zpWord0+1
		bpl falso
		jmp vero
	check_both_positive
		bmi check_a_negative
		bit zpWord0+1
		bmi vero
		jmp vero
	check_a_negative
		bit zpWord0+1
		bpl falso
		jmp vero
	vero
		jmp math.true
	falso
		jmp math.false
	.endproc
	
	;	-------------------------------------- u16 >

	u16_cmp_gt	.proc
		cpy     zpWord0+1
		bne     comparehigh
		cmp     zpWord0
	comparehigh
		bcc     falso
		beq     falso
	vero
		jmp math.true
	falso
		jmp math.false
	.endproc
	
	;	-------------------------------------- u16 >=

	u16_cmp_ge .proc
		cpy     zpWord0+1
		bne     compare_done
		cmp     zpWord0
	compare_done:
		bcc     is_false
	is_true:
		jmp     math.true
	is_false:
		jmp     math.false
	.endproc

	; ##############################################

	; $bc5b = Compare FAC1 with memory contents at A/Y (lo/high)
	; 
	; Returns with A = 
	;     $00 if the values are equal, 
	;     $01 if FAC1 > MEM, or 
	;     $ff if FAC1 < MEM. 
	;	  Uses the vector at $24 to address the variable, 
	;     leaving FAC1 and FAC2 unchanged. 
	
	private_compare .proc
		jsr math.swap_fac1_fac2
		lda #<$ff
		ldy #>$ff
		tax
		jsr basic.copy_fac1_to_mem
		jsr math.swap_fac1_fac2
		lda #<$ff
		ldy #>$ff
		rts
	.endproc
	
	f40_cmp_lt     .proc
		jsr math.private_compare
		jsr basic.fac1_compare
		cmp #$ff		;	<
		bne falso
	vero
		jmp     math.true
	falso
		jmp     math.false
	.pend
	
	f40_cmp_le     .proc
		jsr math.private_compare
		jsr basic.fac1_compare
		cmp #$00		;	>
		beq falso
	vero
		jmp     math.true
	falso
		jmp     math.false
	.pend

	f40_cmp_gt     .proc
		jsr math.private_compare
		jsr basic.fac1_compare
		cmp #$01
		bne falso
	vero
		jmp     math.true
	falso
		jmp     math.false
	.pend

	f40_cmp_ge     .proc
		jsr math.private_compare
		jsr basic.fac1_compare
		cmp #$ff		;	<
		beq falso
	vero
		jmp     math.true
	falso
		jmp     math.false
	.pend
	
	f40_cmp_eq     .proc
		jsr math.private_compare
		jsr basic.fac1_compare
		cmp #$00
		bne falso
	vero
		jmp     math.true
	falso
		jmp     math.false
	.pend

	f40_cmp_ne     .proc
		jsr math.private_compare
		jsr basic.fac1_compare
		cmp #$00
		bne falso
	vero
		jmp     math.false
	falso
		jmp     math.true
	.pend

	;	-------------------------------------- END COMPARE
	
	;	-------------------------------------- begin && ||

	;------------------------------------------------------------ a && y (logico)

	logical_and_u8s8	.proc
		cmp #$00       ; lhs (a) == 0?
		beq false_8
		ldy zpWord0
		cpy #$00       ; rhs (y) == 0?
		beq false_8
		jmp math.true
	false_8:
		jmp math.false
	.endproc

	;------------------------------------------------------------ ay && zpWord0

	logical_and_u16s16	.proc
		cmp #$00       ; lhs low !=0?
		bne true1_16
		cpy #$00       ; lhs high !=0?
		beq false_16   ; lhs==0 -> false
	true1_16:
		lda zpWord0    ; rhs low !=0? (zpWord0 = $00)
		cmp #$00
		bne true2_16
		lda zpWord0+1  ; rhs high !=0? (assumi $01 = zpWord0+1)
		cmp #$00
		beq false_16   ; rhs==0 -> false
	true2_16:
		jmp math.true
	false_16:
		jmp math.false
	.endproc

	;------------------------------------------------------------ a || y (logico)

	logical_or_u8s8	.proc
		cmp #$00
		bne or_8_true
		ldy zpWord0
		cpy #$00
		bne or_8_true
		jmp math.false
		rts
	or_8_true:
		jmp math.true
	.endproc
	
	;------------------------------------------------------------ ay || zpWord0

	logical_or_u16s16	.proc
		cmp #$00
		bne or_16_true
		cpy #$00
		bne or_16_true
		lda zpWord0
		cmp #$00
		bne or_16_true
		lda zpWord0+1
		cmp #$00
		bne or_16_true
		jmp math.false
	or_16_true:
		jmp math.true
	.endproc
	
	;	-------------------------------------- end && ||

;**********
;				BITWISE
;**********

	;------------------------------------------------------------	[v]

	shl_u8	.proc
		ldx zpWord0     ; load shift count into x
		cpx #8          ; compare with 8
		bcc doshift     ; if x < 8, proceed to shift
		lda #0          ; otherwise, result is 0
		rts             ; return
	doshift
		beq done        ; if x == 0, no shift needed
	loop
		asl           	; shift a left by 1
		dex             ; decrement count
		bne loop        ; repeat until x == 0
	done:
		rts             ; return
	.endproc

	shl_s8	.proc
		ldx zpWord0     ; load shift count into x
		cpx #8          ; compare with 8
		bcc doshift     ; if x < 8, proceed to shift
		lda #0          ; otherwise, result is 0
		rts             ; return
	doshift
		beq done        ; if x == 0, no shift needed
	loop
		asl           	; shift a left by 1
		dex             ; decrement count
		bne loop        ; repeat until x == 0
	done:
		rts             ; return
	.endproc

	;------------------------------------------------------------	[]
	
	shl_u16	.proc
		ldx zpWord0+1   ; Check high byte of shift count
		bne zero        ; If non-zero, shift >=256 >=16, result 0
		ldx zpWord0     ; Load low byte of shift count into X
		cpx #16         ; Compare with 16
		bcs zero        ; If X >=16, result 0
	doshift:
		beq done        ; If X == 0, no shift needed
	loop:
		asl	            ; Shift low byte left
		pha             ; Push shifted low to stack
		tya             ; High byte to A
		rol             ; Shift high left, incorporating carry
		tay             ; High back to Y
		pla             ; Pull shifted low back to A
		dex             ; Decrement count
		bne loop        ; Repeat until X == 0
	done:
		rts             ; Return
	zero:
		lda #0          ; Set low byte to 0
		tay             ; Set high byte to 0
		rts             ; Return
	.endproc

	;------------------------------------------------------------	[]

	shl_s16	.proc
		lda zpWord0+1   ; Load high byte of RHS
		bpl check_high  ; If positive (or zero), proceed
		jmp zero        ; If negative, undefined -> return 0
	check_high:
		bne zero        ; If high != 0, shift >=256 >=16, result 0
		ldx zpWord0     ; Load low byte of shift count into X
		cpx #16         ; Compare with 16
		bcs zero        ; If X >=16, result 0
	doshift:
		beq done        ; If X == 0, no shift needed
	loop:
		asl		   		; Shift low byte left
		pha             ; Push shifted low to stack
		tya             ; High byte to A
		rol             ; Shift high left, incorporating carry
		tay             ; High back to Y
		pla             ; Pull shifted low back to A
		dex             ; Decrement count
		bne loop        ; Repeat until X == 0
	done:
		rts             ; Return
	zero:
		lda #0          ; Set low byte to 0
		tay             ; Set high byte to 0
		rts             ; Return
	.endproc

	;------------------------------------------------------------	[v]

	shr_u8	.proc
		ldx zpWord0     ; load shift count into x
		cpx #8          ; compare with 8
		bcc doshift     ; if x < 8, proceed to shift
		lda #0          ; otherwise, result is 0
		rts             ; return
	doshift:
		beq done        ; if x == 0, no shift needed
	loop:
		lsr           	; shift a right by 1 (logical shift, fills with 0)
		dex             ; decrement count
		bne loop        ; repeat until x == 0
	done:
		rts             ; return
	.endproc

	shr_s8	.proc
		ldx zpWord0     ; load shift count into x
		cpx #8          ; compare with 8
		bcc doshift     ; if x < 8, proceed to shift
		cmp #0          ; set flags on original A
		bpl zero        ; if positive or zero, result 0
		lda #$FF        ; if negative, result $FF
		rts             ; return
	zero:
		lda #0          ; result 0
		rts             ; return
	doshift:
		beq done        ; if x == 0, no shift needed
	loop:
		cmp #$80        ; set carry if negative (A >= $80 unsigned)
		ror a           ; arithmetic shift right (ror brings carry into bit 7)
		dex             ; decrement count
		bne loop        ; repeat until x == 0
	done:
		rts             ; return
	.endproc

	;------------------------------------------------------------	[]

.comment
	shr_u16	.proc	;	2^16 = 65536, è più che sufficiente

		sta zpa
		sty zpy
		ldx zpWord0
	-	
		lda zpy
		lsr
		sta zpy
		
		lda zpa
		ror
		sta zpa

		dex
		bne -
		
		lda zpa
		ldy zpy
		rts
		
		.endproc
.endc

	shr_u16	.proc
		sta     zpa     ; copia valore in zero page
		sty     zpy
		; carica shift amount
		lda     zpWord0+1
		bne     big_shift     ; se high >0, shift >=256 >16
		lda     zpWord0+0
		cmp     #16
		bcs     big_shift     ; se low >=16, big shift
		; shift piccolo (0-15)
		tax                    ; x = shift count
		beq     load_result   ; se 0, no shift
	shift_loop:
		; esegui 1-bit lsr
		lda     zpy
		lsr     a              ; shift logico: zero in msb
		sta     zpy
	shift_low:
		lda     zpa
		ror     a              ; shifta low, bit7 = carry (old bit0 high)
		sta     zpa
		dex
		bne     shift_loop
		jmp     load_result
	big_shift:
		; shift >=16: set $0000
		lda     #0
		sta     zpa
		sta     zpy
	load_result:
		lda     zpa     ; carica risultato
		ldy     zpy
		rts
	.endproc

	;------------------------------------------------------------	[]

	shr_s16	.proc	;	2^16 = 65536, è più che sufficiente
		sta     zpa     ; copia valore in zero page
		sty     zpy
		; carica shift amount
		lda     zpWord0+01
		bne     big_shift     ; se high >0, shift >=256 >16
		lda     zpWord0+0
		cmp     #16
		bcs     big_shift     ; se low >=16, big shift
		; shift piccolo (0-15)
		tax                    ; x = shift count
		beq     load_result   ; se 0, no shift
	shift_loop:
		; esegui 1-bit asr
		lda     zpy
		bpl     pos_high
		; negativo
		lsr     a
		ora     #$80
		sta     zpy
		jmp     shift_low
	pos_high:
		; positivo
		lsr     a
		sta     zpy
		shift_low:
		; carry ora contiene old bit0 di high
		lda     zpa
		ror     a              ; shifta low, bit7 = carry (old bit0 high)
		sta     zpa
		dex
		bne     shift_loop
		jmp     load_result
	big_shift:
		; shift >=16: estendi segno
		lda     zpy
		bpl     set_zero
		; negativo: set $ffff
		lda     #$ff
		sta     zpa
		sta     zpy
		jmp     load_result
	set_zero:
		; positivo: set $0000
		lda     #0
		sta     zpa
		sta     zpy
	load_result:
		lda     zpa     ; carica risultato
		ldy     zpy
		rts
	.endproc

	;------------------------------------------------------------	[]

	bit_and_word	.proc
		tax            ; salva a (low)
		tya            ; high in a
		and zpWord0+1  ; & rhs high ($01 = zpWord0+1)
		tay            ; ris high in y
		txa            ; ripristina low
		and zpWord0    ; & rhs low ($00)
		rts
	.endproc

	;------------------------------------------------------------	[]

	bit_or_word	.proc
		tax
		tya
		ora zpWord0+1
		tay
		txa
		ora zpWord0
		rts
	.endproc

	;------------------------------------------------------------	[]

	bit_xor_word	.proc
		tax
		tya
		eor zpWord0+1
		tay
		txa
		eor zpWord0
		rts
	.endproc

	;------------------------------------------------------------	[v]

	bit_neg_word	.proc
		eor #$ff       ; ~ low
		tax
		tya
		eor #$ff       ; ~ high
		tay
		txa
		rts
	.endproc
	
.endproc

;	*****	
;			macro	neg_u8	neg_s8	neg_f40
;	*****

math_neg_u8 .macro
	eor #$ff
	clc
	adc #$01
.endm

math_neg_s8	.macro
	eor #$ff
	clc
	adc #$01
.endm

math_neg_f40 .macro
    lda $67
	eor #$ff
	sta $67
.endm

;**********
;           Arithmetic Pointer
;**********

math_mul_word_by_byte	.macro	;	ay	:= ay * 1
	;
.endm

math_mul_word_by_word	.macro	;	ay	:= ay * 2
	jsr math.mul_word_by_2 
.endm

math_mul_word_by_real	.macro	;	ay	:= ay * 5
	jsr math.mul_word_by_5 
.endm

math_mul_word_by_type	.macro	;	ay	:=  ay * zpWord0
	jsr math.mul_word_by_x 
.endm

;**********
;           CONV
;**********

;	*		u8	u16	s8	s16	f40
;	u8		x	V	V	V	V		
;	u16		V	x	V	V	V	
;	s8		V	V	x	V	V	
;	s16		V	V	V	x	V	
;	f40		V	V	V	V	x	

;	............................................ u8 

cast_from_u8_to_u16	.macro
	ldy	#0
.endmacro

cast_from_u8_to_s8	.macro
	;
.endmacro

cast_from_u8_to_s16	.macro
	ldy #0
.endmacro

cast_from_u8_to_f40	.macro
	jsr basic.conv_u8_to_fac1
.endmacro

;	............................................ u16 

cast_from_u16_to_u8	.macro
	ldy	#0
.endmacro

cast_from_u16_to_s8	.macro
	ldy	#0
.endmacro

cast_from_u16_to_s16	.macro
	;
.endmacro

cast_from_u16_to_f40	.macro	; ay	: aaxxyy00
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

.endmacro	

cast_from_u24_to_f40	.macro	; ayx	  ; aaxxyy00
	; a
	sty zpy
	stx zpx
	ldy zpx
	ldx zpy
	;lda #$ff
	;ldx #$ff
	;ldy #$ff
	sec
	jsr basic.set_mantissa  ; sets mantissa to 00yyxxaa
	jsr basic.set_rest_fac1 ; set rest of FAC1 and JMP to $b8d2

.endmacro

;	............................................ s8 

cast_from_s8_to_u8	.macro
	;
.endmacro

cast_from_s8_to_u16	.macro
	ldy	#0
.endmacro

cast_from_s8_to_s16 .macro
	.block
		ora #$00       ; aggiorna i flag da A (N riflette il bit 7 di A)
		bmi neg        ; se A < 0 (N=1), vai a neg
		ldy #$00       ; A >= 0 → high byte = 0x00
		bne done       ; salta sempre (bne perché Z=0 qui)
	neg
		ldy #$ff       ; A < 0 → high byte = 0xFF
	done
	.endblock
.endmacro

cast_from_s8_to_f40	.macro
	ldy #255
	jsr basic.conv_s16_to_fac1
.endmacro
	
;	............................................ s16
	
cast_from_s16_to_u8	.macro
	ldy #0
.endmacro

cast_from_s16_to_s8	.macro
	ldy #0
.endmacro

cast_from_s16_to_u16	.macro
	;
.endmacro

cast_from_s16_to_f40	.macro
	jsr basic.conv_s16_to_fac1
.endmacro

;	............................................ f40

cast_from_f40_to_u8	.macro
		jsr basic.conv_fac1_to_i32
		lda $65
.endmacro

cast_from_f40_to_s8	.macro
		jsr basic.conv_fac1_to_i32
		lda $65
.endmacro

cast_from_f40_to_u16	.macro
		jsr basic.conv_fac1_to_i32
		lda $65
		ldy $64
.endmacro

cast_from_f40_to_s16	.macro
		jsr basic.conv_fac1_to_i32
		lda $65
		ldy $64
.endmacro

;	............................................ ++ -- 

;	++a		;		a++
;	++ay	;		ay++
;	*ay++	;		*++ay	
;	--a		;		a--
;	--ay	;		ay--
;	*ay--	;		*--ay	

;	............................................ ++ -- BYTE

inc_byte_a	.macro
	tax
	inx
	txa
.endmacro

dec_byte_a	.macro
	tax
	dex
	txa
.endmacro

;	............................................ ++ -- WORD

.comment
inc_word_ay	.macro
	.block
	tax
	inx
	bne	+
	iny
+	
	txa
	.endblock
.endmacro
.endc

inc_word_ay .macro
    .block
		clc         ; azzera carry
		adc #1      ; A = A + 1
		bne no_carry
		iny         ; se low byte è tornato a 0, incrementa high byte
	no_carry
    .endblock
.endmacro

.comment
dec_word_ay	.macro
	.block
    tax
    dex
    bne   +
    dey
+   
	txa
	.endblock
.endmacro
.endc

dec_word_ay .macro
    .block
    sec
    sbc #1        ; decrementa low byte
    bne no_borrow
    dey           ; se low byte è tornato a $FF, decrementa high byte
no_borrow:
    .endblock
.endmacro

;	............................................ ++ -- ptr

inc_ptr_ay	.macro
	.block
		clc		; azzera carry prima dell'addizione
		adc #\1	; somma 5 al low byte
		bcc +	; se non c'è carry, salta
		iny		; se c'è carry, incrementa high byte
	+
	.endblock
.endmacro

dec_ptr_ay    .macro
    .block
	sec			; per sottrarre correttamente, carry deve essere 1
	sbc #\1		; sottrae 5 dal low byte
	bcs +   	; se carry=1, nessun prestito → salta
	dey			; se carry=0, decrementa high byte
	+
    .endblock
.endmacro

;	--------------------------------------------------------------- bit operation

.comment

	unsigned 
		
		<<	u8		:	math_shl_u8
		>>	u8		:	math_shr_u8
		<<	u16		:	math_shl_u16	
		>>	u16		:	math_shr_u16
		%&	u8		:	math_bit_and_byte	
		%&	u16		:	math_bit_and_word		
		%|	u8		:	math_bit_or_byte	
		%|	u16		:	math_bit_or_word
		%^	u8		:	math_bit_xor_byte
		%^	u16		:	math_bit_xor_word
		%~	u8		:	math_bit_neg_byte		(prefix)
		%~	u16		:	math_bit_neg_word		(prefix)
		%-	u8		:	math_bit_neg_byte		(prefix)
		%-	u16		:	math_bit_neg_word		(prefix)
		
	signed

		<<	s8		:	math_shl_s8	
		>>	s8		:	math_shr_s8
		<<	s16		:	math_shl_s16	
		>>	s16		:	math_shr_s16				
		%&	s8		:	math_bit_and_byte	
		%&	s16		:	math_bit_and_word	
		%|	s8		:	math_bit_and_byte	
		%|	s16		:	math_bit_and_word
		%^	s8		:	math_bit_xor_byte
		%^	s16		:	math_bit_xor_word
		%~	s8		:	math_bit_neg_byte		(prefix)
		%~	s16		:	math_bit_neg_word		(prefix)
		%-	s8		:	math_bit_neg_byte		(prefix)
		%-	s16		:	math_bit_neg_word		(prefix)
.endc

;	------------------------------------------	<< u8			[v]	
math_shl_u8		.macro
	jsr math.shl_u8
.endm

;	------------------------------------------	<< s8			[v]	
math_shl_s8		.macro
	jsr math.shl_s8
.endm

;	------------------------------------------	<< u16			[]
math_shl_u16	.macro
	jsr math.shl_u16
.endm

;	------------------------------------------	<< s16			[]
math_shl_s16	.macro
	jsr math.shl_s16
.endm

;	------------------------------------------	>> u8			[]
math_shr_u8	.macro
	jsr math.shr_u8
.endm

;	------------------------------------------	>> s8			[]
math_shr_s8	.macro
	jsr math.shr_s8
.endm

math_shr_u16	.macro	;	[x]
	jsr math.shr_u16
.endm

math_shr_s16	.macro	;	[x]
	jsr math.shr_s16
.endm

;	------------------------------------------	& u8s8			[]
math_bit_and_byte	.macro		;	[v]
	and zpWord0+0
.endm

;	------------------------------------------	& u16s16		[]
math_bit_and_word	.macro		;	[v]
	jsr math.bit_and_word
.endm

;	------------------------------------------	| u8s8			[]
math_bit_or_byte	.macro		;	[v]
	ora zpWord0+0
.endm

;	------------------------------------------	| u16s16		[]
math_bit_or_word	.macro
	jsr math.bit_or_word
.endm

;	------------------------------------------	^ u8s8			[]
math_bit_xor_byte	.macro		;	[v]
	eor zpWord0+0	; a ^= zpy
.endm

;	------------------------------------------	^ u16s16		[]
math_bit_xor_word	.macro		;	[v]
	jsr math.bit_xor_word
.endm

;	------------------------------------------	~ u8s8			[]
math_bit_neg_byte	.macro		;	[v]
	eor #$ff		; ~a
.endm

;	------------------------------------------	~ u16s16			[]
math_bit_neg_word	.macro		;	[v]
	jsr  math.bit_neg_word
.endm

;;;
;;
;


