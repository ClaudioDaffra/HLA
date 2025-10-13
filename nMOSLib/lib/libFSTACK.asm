
;**********
;           fstack	(FAST)
;**********

.comment

	lo stack inizia da MEM_EXTRA_TOP ($CFFF)
	dimensioni 4K					 ($C000)
	dimensioni stack funzione		 ($FF)  Software	utilizzato per le variabili locali
	dimensione expr					 ($FF)	Hardware	utilizzato dal compilatore


.endc

fstack	.proc

	; ### PROGRAM_STACK_SIZE    		=	STACK_SIZE_SHORT  						
	;          								$ff  
	; ### PROGRAM_STACK_ADDRESS     	=	MEM_EXTRA_TOP-PROGRAM_STACK_SIZE		
	;    									$cfff	

    ; ---------------------------------------------------------- fstack.address
	
    .weak
        address     = PROGRAM_STACK_ADDRESS     ;	c64 $cfff
        size    	= PROGRAM_STACK_SIZE    	;	c64	$ff		(short)
    .endweak

    ; ---------------------------------------------------------- fstack.init
	
	init	.proc

		lda #<fstack.address		;	ex $cfff
		sta sp+0
		sta bp+0
		ldy #>fstack.address		;	ex $cfff
		sty sp+1
		sty bp+1
		
		rts
	
	.endproc

	;	...................................................	alloc
	
	alloc	.proc

		sta tmp
		sec             ; prepara il borrow = 0
		
		lda bp+0        ; carica il byte basso
		sbc tmp         ; dimensione massima fstack locale 255 (short)
		sta bp+0        ; scrivi il nuovo byte basso

		lda bp+1        ; carica il byte alto
		sbc #$00        ; sottrai eventuale borrow
		sta bp+1        ; scrivi il nuovo byte alto

		lda bp+0
		sta sp+0
		ldy bp+1
		sty sp+1

		inc sp+0		; 	-1 perchè va allocato il byte
		bne +			;	in sta (sp),y
		inc sp+1		; 	e poi descrementato	
	+
		
		rts
	
	.endproc

	;	...................................................	load effective address bp
	;
	;	input 	: a <- offset
	;	output	: address-> ay	
	;
	
	lea_bp .proc

		sta tmp
		clc	
		lda bp+0
		adc tmp
		tax
		lda bp+1
		adc #0
		tay
		txa

		rts
		
	.endproc

	lea_sp .proc

		sta tmp
		clc	
		lda lea_sp+0
		adc tmp
		tax
		lda lea_sp+1
		adc #0
		tay
		txa

		rts
		
	.endproc
	
	;	.......................................................... push
	
	push	.proc		;	a
	
		byte	.proc
		
			dec sp+0
			bne +
			dec sp+1
		+
			ldy #0
			sta (sp),y
			
			rts
		.endproc

		word	.proc	;	ay
		
			tax
			tya
			
			dec sp+0
			bne +
			dec sp+1
		+
			dec sp+0
			bne +
			dec sp+1
		+
			ldy #0
			sta (sp),y

			iny
			txa
			sta (sp),y
		
			rts
		.endproc
	
		real	.proc
		
			sec
			lda sp+0
			sbc #5
			sta sp+0

			lda sp+1
			sbc #0
			sta sp+1

			lda sp+0
			ldy sp+1
			jsr mem.store.real
			
			rts
			
		.endproc

        fac1_c64    .proc  

            pla            ; backup return address
            sta zpa
            pla
            sta zpy
            
            lda $61
            pha
            lda $62
            pha
            lda $63
            pha
            lda $64
            pha
            lda $65
            pha
            lda $66
            pha
            
            lda zpy        ; restore return address
            pha
            lda zpa
            pha
            
            rts
            
        .endproc
		
	.endproc
	
	;	.......................................................... pop
	
	pop	.proc
	
		byte	.proc

			ldy #0
			lda (sp),y		
			inc sp+0
			bne +
			inc sp+1
		+
			rts
		.endproc

		word	.proc
		
			ldy #1
			lda (sp),y
			tax

			dey
			lda (sp),y
			tay
			txa
			
			inc sp+0
			bne +
			inc sp+1
		+
			inc sp+0
			bne +
			inc sp+1
		+

			rts
		.endproc

		real	.proc

			lda sp+0
			ldy sp+1
			jsr mem.load.real

			clc
			lda sp+0
			adc #5
			sta sp+0

			lda sp+1
			adc #0
			sta sp+1

			rts
			
		.endproc
		
        fac1_c64    .proc  
        
            pla            ; backup return address
            sta zpa
            pla
            sta zpy
            
            pla
            sta $66
            pla
            sta $65
            pla
            sta $64
            pla
            sta $63
            pla
            sta $62
            pla
            sta $61
            
            lda zpy        ; restore return address
            pha
            lda zpa
            pha
            
            rts
			
        .endproc

        fac1_c64_to_fac2    .proc  
        
            pla            ; backup return address
            sta zpa
            pla
            sta zpy
            
            pla
            sta $6e	;	110
            pla
            sta $6d	;	109
            pla
            sta $6c	;	108
            pla
            sta $6b	;	107
            pla
            sta $6a	;	106
            pla
            sta $69	;	105
            
            lda zpy        ;    restore return address
            pha
            lda zpa
            pha
            
            rts
            
        .endproc
		
	.endproc
	
.endproc
	
; -------------------------------------------------- fstack_byte_set , fstack_byte_get

fstack_byte_set .macro

	ldy #\1	
	sta (bp),y

.endmacro

fstack_byte_get .macro

	ldy #\1	
	lda (bp),y

.endmacro

; -------------------------------------------------- fstack_word_set , fstack_word_get

private_fstack_word_set	.proc

	sty zpy
	stx zpx
	ldy zpx
	sta (bp),y
	dey 
	lda zpy
	sta (bp),y
	
	rts

.endproc

fstack_word_set	.macro

	ldx #\1
	jsr	private_fstack_word_set
	
.endmacro

private_fstack_word_get	.proc

	pha
	txa
	tay
	pla
	lda (bp),y
	tax
	dey
	lda (bp),y
	tay
	txa
	
	rts
	
.endproc

fstack_word_get	.macro

	ldx #\1
	jsr private_fstack_word_get
	
.endmacro

; -------------------------------------------------- fstack_fac1_set , fstack_fac1_get

fstack_fac1_set	.macro
	lda #(\1-4)
	jsr fstack.lea_bp	; ->	ay
	jsr basic.store_fac1
.endmacro

fstack_fac1_get	.macro
	lda #(\1-4)
	jsr fstack.lea_bp	; ->	ay
	jsr basic.load5_fac1
.endmacro

.comment
fstack_real_set	.macro

	jsr mem.load.real
	lda #(\1-4)
	jsr fstack.lea_bp
	jsr mem.store.real
	
.endmacro

fstack_real_get	.macro

	lda #(\1-4)
	jsr fstack.lea_bp
	jsr mem.load.real
	
.endmacro
.endc

;

fstack_push_byte_fast	.macro
	pha
.endmacro

fstack_pop_byte_fast	.macro
	pla
.endmacro

fstack_push_word_fast	.macro
	pha
	tya
	pha
.endmacro

fstack_pop_word_fast	.macro
	pla
	tay
	pla
.endmacro

fstack_push_real_fast	.macro

	.block
		ldy #0
	-
		lda $61,y
		pha
		iny
		cpy #5
		bne -
	.endblock
	
.endmacro

fstack_pop_real_fast	.macro

	.block
		ldx #4
	-
		pla
		sta $61,x
		dex
		bpl -
	.endblock
	
.endmacro

fstack_pop_real_fast_in_fac2	.macro

	.block
		ldx #4
	-
		pla
		sta $6E,x
		dex
		bpl -
	.endblock
	
.endmacro

fstack_address_hardware	.macro
	.block

		tsx
		txa
		ldy #>$0100

	.endblock
.endmacro

fstack_push_sp_bp	.macro
	lda sp+0
	pha
	lda sp+1
	pha
	lda bp+0
	pha
	lda bp+1
	pha
.endmacro

fstack_pop_sp_bp	.macro
	pla
	sta bp+1
	pla
	sta bp+0
	pla
	sta sp+1
	pla
	sta sp+0
.endmacro

fstack_push_sp .macro
	lda sp+0
	pha
	lda sp+1
	pha
.endmacro

fstack_pop_sp	.macro
	pla
	sta sp+1
	pla
	sta sp+0
.endmacro

mstack_push_fac1 .macro
	lda $61	;	97
	pha
	lda $62	;	98
	pha
	lda $63	;	99
	pha
	lda $64	;	100
	pha
	lda $65	;	101
	pha
	lda $66	;	102
	pha
.endm

mstack_pop_fac1 .macro
	pla
	sta $66	;	102
	pla
	sta $65	;	101
	pla
	sta $64	;	100
	pla
	sta $63 ;	99
	pla
	sta $62 ;	98
	pla
	sta $61 ;	97
.endm

mstack_pop_fac2 .macro
	pla
	sta $6e	;	110
	pla
	sta $6d	;	109
	pla
	sta $6c	;	108
	pla
	sta $6b	;	107
	pla
	sta $6a	;	106
	pla
	sta $69	;	105
.endm

mstack_pop_byte	.macro
	pla
	sta \1
.endmacro

mstack_pop_word	.macro
	pla
	tay
	sty \1+1
	pla
	sta \1+0
.endmacro

;;;
;;
;

 