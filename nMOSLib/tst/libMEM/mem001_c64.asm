

; .............. commodore 64

; .............. compile : sh casm.sh tst/libMEM/mem001_c64

.include "../../lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../../lib/libCD_c64.asm"

kLocalAsmReal00                     .byte  $81,$33,$33,$33,$33	;	// 1.4
kLocalAsmReal01                     .byte  $82,$4c,$cc,$cc,$cc	;   // 3.2
kLocalAsmReal03                   	.byte  $8b,$00,$18,$93,$74 	;  //  1024.768
kLocalAsmReal04                   	.byte  $8b,$80,$18,$93,$74 	;  // -1024.768
message								.null  "the end"
 
main .proc

	;	..........................	init 1
	
	#load_addr_zpWord0	$c000
	#load_imm_ay		$c002
	#store_word

	;	..........................	store byte ptr
	
	#load_addr_zpWord0	$c000
	lda #12
	jsr mem.store.byte_ptr

	;	..........................	load byte ptr
	
	#load_addr_zpWord0	$c000
	jsr mem.load.byte_ptr
	
	jsr std.print_u8_dec

	;	..........................	init 2
	
	#load_addr_zpWord0	$c000
	#load_imm_ay		$c002
	#store_word

	;	..........................	store word ptr
	
	#load_addr_zpWord0	$c000
	#load_imm_ay		1234
	jsr mem.store.word_ptr

	;	..........................	load word ptr
	
	#load_addr_zpWord0	$c000
	jsr mem.load.word_ptr
	
	jsr std.print_u16_dec

	;	..........................	store real ptr

	#load_addr_zpWord0	$c000
	#load_fac1			kLocalAsmReal04	;	-1024.768
	jsr mem.store.real_ptr

	#load_addr_zpWord0	$c000
	jsr mem.store.real_ptr

	jsr std.print_fac1

	; ------------------------------------ PROVA ????
	
	#load_addr_zpWord0	$c000
	#load_imm_ay		$c002
	#store_word
	;	49152	a	2		$02
	;   49153	y	192		$c0

	#load_addr_ay $c000
	#load_word
	jsr std.print_u16_dec

	rts
 
.endproc

