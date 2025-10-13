

; .............. commodore 64

; .............. compile : sh casm.sh tst/libFSTACK/stack005_c64

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

sub1 .proc	; void sub1 ( byte b1,word w2,real r3  ) {
;	byte b2
;	word w2
;	real r2
;}  
	fstack_local .struct ;	....................................	variabili locali
		b1    .byte 0
		w1    .word 0
		r1    .byte 0,0,0,0,0
		b2    .byte 0
		w2    .word 0
		r2    .byte 0,0,0,0,0
	.endstruct 
	.cerror(size(fstack_local)>fstack.size)	;	................	check fstack size
	.weak	;	................................................	offset nomi
		b1    =	(size(fstack_local)-(fstack_local.b1))
		w1    =	(size(fstack_local)-(fstack_local.w1))
		r1    =	(size(fstack_local)-(fstack_local.r1))
		b2    =	(size(fstack_local)-(fstack_local.b2))
		w2    =	(size(fstack_local)-(fstack_local.w2))
		r2    =	(size(fstack_local)-(fstack_local.r2))
	.endweak
	
;	salva bp e sp
#fstack_push_sp_bp
	
	;	alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc
	
	;-------------------------	body
		
	;	set parameter
	
			;............................... b byte
			
			lda #12
			fstack_byte_set b2

			;............................... w word
			lda #<49154
			ldy #>49154
			fstack_word_set w2

			;............................... r real
			lda #<kLocalAsmReal00
			ldy #>kLocalAsmReal00
			fstack_real_set r2
				
	;	get parameter
	
			std_print_nl
			fstack_byte_get b1
			jsr std.print_u8_dec

			std_print_nl
			fstack_word_get w1
			jsr std.print_u16_dec

			std_print_nl
			fstack_real_get r1
			jsr std.print_fac1	

	;	get parameter local

			std_print_nl
			fstack_byte_get b2
			jsr std.print_u8_dec

			std_print_nl
			fstack_word_get w2
			jsr std.print_u16_dec

			std_print_nl
			fstack_real_get r2
			jsr std.print_fac1	
			
	;-------------------------

			std_print_nl

;	ripristina bp e sp
#fstack_pop_sp_bp
	
rts
	
.endproc

; ############################################### MAIN
; ############################################### MAIN
; ############################################### MAIN
	
main .proc 
; void sub1 ( byte argc, argv )  {
;	byte c
;	real d
;	byte e
;	word f
;	real g
;	byte h
;	[10] i
;	byte l
; }
	fstack_local .struct ;	....................................	variabili locali
		argc    .byte 0
		argv    .word 0
		c       .byte 0
		r       .byte 0,0,0,0,0
		b       .byte 0
		w       .word 0
		g       .byte 0,0,0,0,0
		h		.byte 0
		i		.fill 10
		l		.byte 0
	.endstruct 
	.cerror(size(fstack_local)>fstack.size)	;	................	check fstack size
	.weak	;	................................................	offset nomi
		argc    =	(size(fstack_local)-(fstack_local.argc))
		argv      =	(size(fstack_local)-(fstack_local.argv))
		c       =	(size(fstack_local)-(fstack_local.c))
		r       =	(size(fstack_local)-(fstack_local.r))
		b       =	(size(fstack_local)-(fstack_local.b))
		w       =	(size(fstack_local)-(fstack_local.w))
		g       =	(size(fstack_local)-(fstack_local.g))
		h       =	(size(fstack_local)-(fstack_local.h))
		i       =	(size(fstack_local)-(fstack_local.i))
		l       =	(size(fstack_local)-(fstack_local.l))
	.endweak

;	salva bp e sp
#fstack_push_sp_bp
	
	;	alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	;-------------------------
	
	;	param init

				;............................... b byte
				
				lda #11
				fstack_byte_set b

				;............................... w word
				lda #<49153
				ldy #>49153
				fstack_word_set w

				;............................... r real
				lda #<kLocalAsmReal04
				ldy #>kLocalAsmReal04
				fstack_real_set r
		
		;	param push
		#fstack_push_sp
		
			fstack_byte_get b
			jsr fstack.push.byte

			fstack_word_get w
			jsr fstack.push.word
			
			fstack_real_get r
			jsr fstack.push.real	
		
		#fstack_pop_sp
		
		jsr sub1
	
	; output param
	
	std_print_nl
	fstack_byte_get b
	jsr std.print_u8_dec
	
	std_print_nl
	fstack_word_get w
	jsr std.print_u16_dec

	std_print_nl
	fstack_real_get r
	jsr std.print_fac1

	;-------------------------
	
;	ripristina bp e sp
#fstack_pop_sp_bp
	
rts
	
.endproc

