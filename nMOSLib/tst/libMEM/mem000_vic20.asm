

; .............. commodore 64

; .............. compile : sh casm.sh tst/libMEM/mem000_vic20

.include "../../lib/libCD.asm"

PROGRAM TARGET_VIC20, PROGRAM_ADDRESS_VIC20 , 2025

.include "../../lib/libCD_vic20.asm"

kLocalAsmReal00                     .byte  $81,$33,$33,$33,$33	;	// 1.4
kLocalAsmReal01                     .byte  $82,$4c,$cc,$cc,$cc	;   // 3.2
kLocalAsmReal03                   	.byte  $8b,$00,$18,$93,$74 	;  //  1024.768
kLocalAsmReal04                   	.byte  $8b,$80,$18,$93,$74 	;  // -1024.768
message								.null  "the end"
 
main .proc

	ADDRESS = 7371	;	$1cb7
	
	; ########################################################## MEM
	
	; copy short
	
		load_addr_zpWord0	kLocalAsmReal00		;	1.4		source
		load_addr_zpWord1	kLocalAsmReal01		;	3.2		dest
		lda #mem.sizeof.f40
		jsr mem.copy.short
		
		load_fac1 kLocalAsmReal01				;	1.4
		jsr std.print_fac1

	; ########################################################## LOAD
	
	; load byte
	
		lda #01		;	256 $0101
		ldy #01
		
		
		lda #<ADDRESS
		ldy #>ADDRESS

		sta ADDRESS
		sty ADDRESS+1
		
		#load_addr_ay ADDRESS
		jsr mem.load.byte
		jsr std.print_u8_dec					;	0,192

		#load_addr_ay ADDRESS
		#load_byte
		jsr std.print_u8_dec					;	0,192

	; load word
	
		load_addr_ay ADDRESS
		jsr mem.load.word
		jsr std.print_u16_dec					;	4092
	
		#load_addr_ay ADDRESS
		#load_word	
		jsr std.print_u16_dec					;	4092
	
	; load real
	
		#load_addr_ay kLocalAsmReal04
		jsr mem.load.real
		jsr std.print_fac1						; -1024.768

	lda #petscii.nl
	jsr kernel.chrout
	
	; ....................................	PAGE ZERO 
	
		lda #01		;	256 $0101
		ldy #01
		sta 253
		sty 254
	
	;	#zload_byte
	
		lda #253
		#zload_byte
		jsr std.print_u8_dec					;	1

		lda #01		;	256 $0101
		ldy #01
		sta 253
		sty 254
	
	;	#zload_word
	
		lda #253
		#zload_word
		jsr std.print_u16_dec					;	257
.comment
	;	#zload_real ???

				load_addr_zpWord0	kLocalAsmReal00		;	1.4		source
				;	load_addr_zpWord1	
				;   235	valore ko SCREEN ADDRESS LINE BASIC 217-242
				load_addr_zpWord1	243
				lda #mem.sizeof.f40
				jsr mem.copy.short

				;lda #235 valore ko
				lda #243
				#zload_real
				jsr std.print_fac1
.endcomment

	lda #petscii.nl
	jsr kernel.chrout
	
	; ########################################################## store

	;	store byte
	
		load_addr_ay	ADDRESS
		ldx	#71
		#store_byte
		
		lda ADDRESS
		jsr std.print_u8_dec			;	71

	;	zstore byte
	
		ldx #72
		lda #250
		#zstore_byte					;	72

		lda 250
		jsr std.print_u8_dec

	;	store word

		#load_addr_zpWord0	ADDRESS		;	5678
		#load_imm_ay		5678
		#store_word
		lda ADDRESS
		ldy ADDRESS+1
		jsr std.print_u16_dec

	;	zstore word

		ldx #250
		#load_imm_ay		4321
		#zstore_word

		lda 250
		ldy 251
		jsr std.print_u16_dec			; 4321

		lda #petscii.nl
		jsr kernel.chrout
	
	;	store real
	
		#load_addr_ay kLocalAsmReal04
		jsr mem.load.real
		#load_addr_ay ADDRESS
		#store_real
		
		#load_addr_ay ADDRESS
		jsr mem.load.real
		jsr std.print_fac1

		;	############################################ store
		
		lda #petscii.nl
		jsr kernel.chrout
			
		;	store byte
		
		load_addr_ay ADDRESS
		ldx #123
		jsr mem.store.byte
		lda ADDRESS
		jsr std.print_u8_dec				 ;	123

		;	store word
		
		load_addr_zpWord0 ADDRESS
		load_imm_ay 	  5678
		jsr mem.store.word
		lda ADDRESS
		ldy ADDRESS+1
		jsr std.print_u16_dec				;	5678

		;	store real
		
		load_fac1 		  kLocalAsmReal04
		load_imm_ay 	  ADDRESS
		jsr mem.store.real
		load_fac1 		  ADDRESS
		jsr std.print_fac1					; -1024.768

		lda #petscii.nl
		jsr kernel.chrout



	rts
 
.endproc

 