

; .............. commodore 64

; .............. compile : sh casm.sh tst/libFSTACK/stack003_c64

.include "../../lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../../lib/libCD_c64.asm"

kLocalAsmReal00                     .byte  $81,$33,$33,$33,$33	;	// 1.4
kLocalAsmReal01                     .byte  $82,$4c,$cc,$cc,$cc	;   // 3.2
kLocalAsmReal03                   	.byte  $8b,$00,$18,$93,$74 	;  //  1024.768
;										   139 128  24 147 116
kLocalAsmReal04                   	.byte  $8b,$80,$18,$93,$74 	;  // -1024.768
message								.null  "the end"

; ###############################################  
; ############################################### LIB  
; ###############################################  


; ############################################### SUB1
; ############################################### SUB1
; ############################################### SUB1

sub1 .proc	; void sub1 ( byte,word,real,byte ) { word real }  

	fstack_local .struct ;	....................................	variabili locali
		; (
		aa      .byte ?
		bb      .word ?
		cc      .byte ?,?,?,?,?
		dd      .byte ?
		; )
		ee      .word ?
		ff      .byte ?,?,?,?,?
	.endstruct 
	.cerror(size(fstack_local)>fstack.size)	;	................	check fstack size
	.weak	;	................................................	offset nomi
		aa	=	(size(fstack_local)-(fstack_local.aa))
		bb	=	(size(fstack_local)-(fstack_local.bb))
		cc	=	(size(fstack_local)-(fstack_local.cc))
		dd	=	(size(fstack_local)-(fstack_local.dd))
		ee	=	(size(fstack_local)-(fstack_local.ee))
		ff	=	(size(fstack_local)-(fstack_local.ff))
	.endweak

	;	.................................... print fstack sp/bp address
	
	lda #petscii.nl
	jsr kernel.chrout
	
	lda #size(fstack_local)
	jsr std.print_u8_dec

	lda #petscii.space
	jsr kernel.chrout

	lda #petscii.space
	jsr kernel.chrout
	
	lda sp+0
	ldy sp+1
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout
	
	lda bp+0
	ldy bp+1
	jsr std.print_u16_dec

	;	.................................... fstack_push
	
	VEDI 005 ; fstack_push

	;	.................................... fstack.alloc
	
	lda #size(fstack_local)
	jsr fstack.alloc

	lda #petscii.space
	jsr kernel.chrout
	
	lda sp+0
	ldy sp+1
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout
	
	lda bp+0
	ldy bp+1
	jsr std.print_u16_dec

	;	.................................... param get

	
	;	.................................... fstack.pop
	
	fstack_pop	
	
	rts
	
.endproc

; ############################################### MAIN
; ############################################### MAIN
; ############################################### MAIN
	
main .proc

	fstack_local .struct ;	....................................	variabili locali
		a       .byte 0
		b       .word 0
		c       .byte 0
		d       .byte 0,0,0,0,0
		e       .byte 0
		f       .word 0
		g       .byte 0,0,0,0,0
		h		.byte 0
		i		.fill 10
		l		.byte 0
	.endstruct 
	.cerror(size(fstack_local)>fstack.size)	;	................	check fstack size
	.weak	;	................................................	offset nomi
		a       =	(size(fstack_local)-(fstack_local.a))
		b       =	(size(fstack_local)-(fstack_local.b))
		c       =	(size(fstack_local)-(fstack_local.c))
		d       =	(size(fstack_local)-(fstack_local.d))
		e       =	(size(fstack_local)-(fstack_local.e))
		f       =	(size(fstack_local)-(fstack_local.f))
		g       =	(size(fstack_local)-(fstack_local.g))
		h       =	(size(fstack_local)-(fstack_local.h))
		i       =	(size(fstack_local)-(fstack_local.i))
		l       =	(size(fstack_local)-(fstack_local.l))
	.endweak

	;	.................................... print fstack sp/bp address
	;	.................................... print fstack sp/bp address

	lda #size(fstack_local)
	jsr std.print_u8_dec

			lda #petscii.space
			jsr kernel.chrout
			
			lda sp+0
			ldy sp+1
			jsr std.print_u16_dec		;	53247

			lda #petscii.space
			jsr kernel.chrout
			
			lda bp+0
			ldy bp+1
			jsr std.print_u16_dec		;	53247

			lda #petscii.space
			jsr kernel.chrout
	;	.................................... fstack_push
	
	fstack_push
	
	;	.................................... print fstack sp/bp address
	
	;	.................................... fstack.alloc
 
	lda #size(fstack_local)
	jsr fstack.alloc
	
	lda #petscii.nl
	jsr kernel.chrout
	
			lda #size(fstack_local)
			jsr std.print_u8_dec

			lda #petscii.space
			jsr kernel.chrout
			
			lda sp+0
			ldy sp+1
			jsr std.print_u16_dec		;	53247

			lda #petscii.space
			jsr kernel.chrout
			
			lda bp+0
			ldy bp+1
			jsr std.print_u16_dec		;	53247

			lda #petscii.space
			jsr kernel.chrout


	;	*********
	;				param push
	;	*********
	
	lda #98
	fstack_push_byte_fast

	lda #<513
	ldy #>513
	fstack_push_word_fast

	lda #97
	fstack_push_byte_fast

	lda #<514
	ldy #>514
	fstack_push_word_fast

	; 139 128  24 147 116	;	-1024.768
	lda #<kLocalAsmReal04
	ldy #>kLocalAsmReal04
	jsr mem.load.real
	fstack_push_real_fast

	;	*********
	;				param pop
	;	*********	

    lda #petscii.nl
	jsr kernel.chrout

	fstack_pop_real_fast
	jsr std.print_fac1
	
	lda #petscii.space
	jsr kernel.chrout
	
	fstack_pop_word_fast
	jsr std.print_u16_dec
	
	lda #petscii.space
	jsr kernel.chrout
	
	fstack_pop_byte_fast
	jsr std.print_u8_dec
	
	lda #petscii.space
	jsr kernel.chrout
	
	fstack_pop_word_fast
	jsr std.print_u16_dec
	
	lda #petscii.space
	jsr kernel.chrout
	
	fstack_pop_byte_fast
	jsr std.print_u8_dec

	;	.................................... fstack.pop

	lda #petscii.nl
	jsr kernel.chrout
	
	fstack_pop	
	
	rts
	
.endproc

