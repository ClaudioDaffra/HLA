

; .............. commodore 64

; .............. compile : sh casm.sh tst/libFSTACK/stack000_c64

.include "../../lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../../lib/libCD_c64.asm"

kLocalAsmReal00                     .byte  $81,$33,$33,$33,$33	;	// 1.4
kLocalAsmReal01                     .byte  $82,$4c,$cc,$cc,$cc	;   // 3.2
kLocalAsmReal03                   	.byte  $8b,$00,$18,$93,$74 	;  //  1024.768
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
		dd	=	(size(fstack_local)-(fstack_local.aa))
		ee	=	(size(fstack_local)-(fstack_local.bb))
		ff	=	(size(fstack_local)-(fstack_local.cc))
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
		a       .byte ?
		b       .word ?
		c       .byte ?
		d       .byte ?,?,?,?,?
		e       .byte ?
		f       .word ?
		g       .byte ?,?,?,?,?
		h		.byte ?
		i		.fill 10
		l		.byte ?
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

	;	.................................... fstack.alloc
 
	lda #size(fstack_local)
	jsr fstack.alloc
	
	lda #petscii.nl
	jsr kernel.chrout
	
	;	.................................... param print position

	lda #petscii.space
	jsr kernel.chrout

	ldy #a
	tya
	jsr std.print_u8_dec
 
	lda #petscii.space
	jsr kernel.chrout

	ldy #b
	tya
	jsr std.print_u8_dec

	lda #petscii.space
	jsr kernel.chrout

	ldy #c
	tya
	jsr std.print_u8_dec
 
	lda #petscii.space
	jsr kernel.chrout

	ldy #d
	tya
	jsr std.print_u8_dec

	lda #petscii.space
	jsr kernel.chrout

	ldy #e
	tya
	jsr std.print_u8_dec
 
	lda #petscii.space
	jsr kernel.chrout

	ldy #f
	tya
	jsr std.print_u8_dec

	lda #petscii.space
	jsr kernel.chrout

	ldy #g
	tya
	jsr std.print_u8_dec
 
	lda #petscii.space
	jsr kernel.chrout

	ldy #h
	tya
	jsr std.print_u8_dec

	lda #petscii.space
	jsr kernel.chrout

	ldy #i
	tya
	jsr std.print_u8_dec

	lda #petscii.space
	jsr kernel.chrout

	ldy #l
	tya
	jsr std.print_u8_dec
	
	lda #petscii.nl
	jsr kernel.chrout

	
	;	.................................... param print address

	lda #a
	jsr fstack.lea_bp
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout

	lda #b
	jsr fstack.lea_bp
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout

	lda #c
	jsr fstack.lea_bp
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout

	lda #d
	jsr fstack.lea_bp
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout

	lda #e
	jsr fstack.lea_bp
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout

	lda #f
	jsr fstack.lea_bp
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout

	lda #g
	jsr fstack.lea_bp
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout

	lda #h
	jsr fstack.lea_bp
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout

	lda #i
	jsr fstack.lea_bp
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout

	lda #l
	jsr fstack.lea_bp
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout
	
	; ...

	lda #petscii.nl
	jsr kernel.chrout
	
	;	.................................... param set
 
 
	jsr sub1
	
	;	.................................... fstack.pop
	
	fstack_pop	
	
	rts
	
.endproc

