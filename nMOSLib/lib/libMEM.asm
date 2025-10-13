
;**********
;           MEM
;**********

.comment

	mem.sizeof
	mem.copy.short

load

	mem.load.byte			;	a 			:= (ay)
	mem.load.word			;	ay 			:= (ay)
	mem.load.real			;	fac1 		:= (ay)

	mem.load.byte_ptr		;	a 			:= (*zpWord0)
	mem.load.word_ptr		;	ay 			:= (*zpWord0)
	mem.load.real_ptr		;	fac1		:= (*zpWord0)
	
	load_byte 				;	a 			:= (ay)
	load_word				;	ay 			:= (ay)
	load_real 				;	fac1 		:= (ay)

	zload_byte 				;	a			:= (a)
	zload_word 				;	ay			:= (a)	
	zload_real				;	fac1 		:= (a)	!! carefully Page Zero Screen !!

store

	mem.store.byte			;	(ay)  		:= x
	mem.store.word			;	(zpWord0)  	:= ay
	mem.store.real			;	(ay)  		:= fac1

	mem.store.byte_ptr		;	(*zpWord0)  := a
	mem.store.word_ptr		;	(*zpWord0)  := ay
	mem.store.real_ptr		;	(*zpWord0)  := fac1

	store_byte 				;	(ay) 		:= (x)
	store_word 				;	(zpWord0) 	:= (ay)
	store_real 				;	(ay)		:= (fac1)
	
	zstore_byte 			;	(a) 		:= (x)
	zstore_word 			;	(x) 		:= (ay
	zstore_real 			;	(a) 		:= (fac1)

.endc

mem	.proc

	sizeof	.proc
	
		f40	 =	SIZEOF_REAL
		fac1 =	SIZEOF_REAL
		fac2 =	SIZEOF_REAL
		word =	SIZEOF_WORD
		byte =	SIZEOF_BYTE
	
	.endproc

	; ---------------------------------------------------------- mem.copy.short
	; input :	a 		size
	; 			zpWord0	source
	;			zpWord1	dest
	;
	copy	.proc
		short	.proc
			sta mem.copy.short.size+1
			ldy #0
		-
			lda (zpWord0),y
			sta (zpWord1),y
			iny
		size
			cpy #5	; self-modified(5) 
			bne -
			rts
		.endproc
	.endproc

	; ---------------------------------------------------------- load
	
	load	.proc

	; ---------------------------------------------------------- load byte
	
		byte	.proc		;	a := (ay)
			sta zpWord0+0
			sty zpWord0+1
		zpWord
			ldy #0
			lda (zpWord0),y
			rts
		.endproc
	
	; ---------------------------------------------------------- load word
	
		word	.proc		;	ay := (ay)
			sta zpWord0+0
			sty zpWord0+1
		zpWord
			ldy #0
			lda (zpWord0),y
			tax
			iny
			lda (zpWord0),y
			tay
			txa
			rts
			
		.endproc

	; ---------------------------------------------------------- load real
	
		real .proc				;	fac1 := (ay)
			jsr basic.load5_fac1
			rts
		.endproc

	; ---------------------------------------------------------- byte ptr
	
		byte_ptr .proc			;	 a  := (*ay)
			jsr mem.load.word
			jsr mem.load.byte
			rts
		.endproc

	; ---------------------------------------------------------- word ptr
	
		word_ptr .proc			;	ay := (*ay)
			jsr mem.load.word
			jsr mem.load.word
			rts
		.endproc

	; ---------------------------------------------------------- real ptr
	
		real_ptr .proc			;	fac1 := (*ay)
			jsr mem.load.word
			jsr mem.load.real
			rts
		.endproc
		
	.endproc	; end load

	; ---------------------------------------------------------- store
	
	store	.proc
	
		byte	.proc			;	 (ay)  := x
			sta putx+1
			sty putx+2
		putx
			stx $ffff
			rts
			
		.endproc

		word	.proc			;	 (zpWord0)  := ay
			tax
			sty zpy
			ldy #0
			txa
			sta (zpWord0),y
			lda zpy
			iny
			sta (zpWord0),y
			rts
		.endproc

		real	.proc			;	 (ay)  := fac1

			jsr basic.store_fac1
			rts

		.endproc

		byte_ptr	.proc		;	 (*zpWord0)  := a

			pha
			jsr mem.load.word.zpWord
			sta zpWord0
			sty zpWord0+1
			ldy #0
			pla
			sta (zpWord0),y
			rts
			
		.endproc

		word_ptr	.proc			;	 (*zpWord0)  := ay

			pha
			tya
			pha
			jsr mem.load.word.zpWord
			sta zpWord0
			sty zpWord0+1
			pla
			tay
			pla
			jsr mem.store.word

			rts
			
		.endproc

		real_ptr	.proc		;	(*zpWord0) := fac1

			jsr mem.load.word.zpWord
			jsr basic.store_fac1
			rts
			
		.endproc

	.endproc	; end store

.endproc	; end mem

;	..................................................... macro

;	# LOAD

load_byte	.macro		;	a 		:= (ay)
	.block
		sta geta+1
		sty geta+2
	geta
		lda $ffff
	.endblock
.endmacro


load_word	.macro		;	ay 		:= (ay)
		jsr mem.load.word
.endmacro

load_real	.macro		;	fac1 	:= (ay)
		jsr basic.load5_fac1
.endmacro

zload_byte	.macro		;	a		:= (a)
	.block
		sta geta+1
	geta
		lda $ff
	.endblock
.endmacro

zload_word	.macro		;	ay		:= (a)		
	.block
		sta geta+1
		sta gety+1
		inc gety+1
	geta
		lda $ff
	gety	
		ldy $ff
	.endblock
.endmacro

zload_real	.macro		;	fac1 	:= (a)
	.block
		ldy #0
		jsr basic.load5_fac1
	.endblock
.endmacro

;	# STORE

store_byte	.macro			;		(ay) := (x)	 
		sta putay+1
		sty putay+2
		txa
	putay	
		sta $ffff
.endmacro

store_word	.macro			;		(zpWord0) := ay
		sty zpy
		ldy #0
		sta (zpWord0),y
		lda zpy
		iny
		sta (zpWord0),y
.endmacro

store_real	.macro			;		(ay)	:=	fac1
	jsr basic.store_fac1
.endmacro

zstore_byte	.macro			;		(a) := (x)	
		.block
		sta puta+1
	puta
		stx $ff
		.endblock
.endmacro

zstore_word	.macro			;		(x) := ay	 
		stx putxlo+1
		inx
		stx putxhi+1
	putxlo
		sta $ff
	putxhi
		sty $ff
.endmacro

zstore_real	.macro
	ldy #0
	jsr basic.store_fac1	;		(a) := fac1
.endmacro

; ------------------------------------------------------- BEGIN POINTER

mem_store_byte_pointer	.macro
	ldy #0
	sta (zpWord0),y
.endm

mem_load_byte_pointer	.macro
	sta zpWord0+0
	sty zpWord0+1
	ldy #0
	lda (zpWord0),y
.endm

.comment
mem_store_word_pointer	.macro TODO CHECK NEW VERSION
	tax
	tya
	pha
	ldy #0
	txa
	sta (zpWord0),y
	pla
	iny
	sta (zpWord0),y
.endm
.endc

mem_store_word_pointer	.macro 
	tax
	tya
	ldy #1
	sta (zpWord0),y
	txa
	dey
	sta (zpWord0),y
.endm

mem_load_word_pointer	.macro
	sta zpWord0+0
	sty zpWord0+1
	ldy #0
	lda (zpWord0),y
	tax
	iny
	lda (zpWord0),y
	tay
	txa
.endm

mem_store_real_pointer	.macro
	lda zpWord0+0
	ldy zpWord0+1   
	jsr basic.store_fac1
.endm

mem_load_real_pointer	.macro
	jsr basic.load5_fac1
.endm

; ------------------------------------------------------- END POINTER
	
;
	