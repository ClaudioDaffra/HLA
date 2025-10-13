

; .............. commodore 64

; .............. compile : sh casm.sh tst/libMEM/mem001_vic20

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

	;	..........................	init 1
	
	#load_addr_zpWord0	ADDRESS
	#load_imm_ay		ADDRESS+2
	#store_word

	;	..........................	store byte ptr
	
	#load_addr_zpWord0	ADDRESS
	lda #12
	jsr mem.store.byte_ptr			;	12

	;	..........................	load byte ptr
	
	#load_addr_zpWord0	ADDRESS
	jsr mem.load.byte_ptr
	
	jsr std.print_u8_dec

	;	..........................	init 2
	
	#load_addr_zpWord0	ADDRESS
	#load_imm_ay		ADDRESS+2
	#store_word

	;	..........................	store word ptr
	
	#load_addr_zpWord0	ADDRESS
	#load_imm_ay		1234
	jsr mem.store.word_ptr			;	1234

	;	..........................	load word ptr
	
	#load_addr_zpWord0	ADDRESS
	jsr mem.load.word_ptr
	
	jsr std.print_u16_dec

	;	..........................	store real ptr

	#load_addr_zpWord0	ADDRESS
	#load_fac1			kLocalAsmReal04	;	-1024.768
	jsr mem.store.real_ptr

	#load_addr_zpWord0	ADDRESS
	jsr mem.store.real_ptr

	jsr std.print_fac1					;	-1024.768

	; ------------------------------------ PROVA ????
	
	#load_addr_zpWord0	ADDRESS
	#load_imm_ay		ADDRESS+2
	#store_word
	;	49152	a	2		$02	-> vic 20	$b7	183
	;   49153	y	192		$c0	-> vic 20	$1c	 28

	#load_addr_ay ADDRESS
	#load_word
	jsr std.print_u16_dec			;	7373

	rts
 
.endproc

