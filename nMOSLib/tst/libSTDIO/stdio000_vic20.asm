
; .............. compile : sh casm.sh tst/libSTDIO/stdio000_vic20

.include "../../lib/libCD.asm"

PROGRAM TARGET_VIC20, PROGRAM_ADDRESS_VIC20 , 2025

.include "../../lib/libCD_vic20.asm"

kLocalAsmReal00                     .byte  $81,$33,$33,$33,$33	;	// 1.4
kLocalAsmReal01                     .byte  $82,$4c,$cc,$cc,$cc	;   // 3.2
kLocalAsmReal03                   	.byte  $8b,$00,$18,$93,$74 	;  //  1024.768
kLocalAsmReal04                   	.byte  $8b,$80,$18,$93,$74 	;  // -1024.768
message								.null  "The End"
messageErr							.null  "error too long : abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz abcdefeghilmnopqrstuvz"

main .proc

	lda	#petscii.minuscolo
	jsr kernel.chrout
	
	;	..........................	std.print_uword
	
	load_imm_ay 32768
	jsr std.print_uword
	
	lda #petscii.space
	jsr kernel.chrout

	;	..........................	std.print_fac1
	
	load_fac1 kLocalAsmReal04
	jsr std.print_fac1
	
	lda #petscii.space
	jsr kernel.chrout

	;	..........................	print_u8_dec

	lda #8
	jsr std.print_u8_dec

	lda #petscii.space
	jsr kernel.chrout

	;	..........................	print_s8_dec

	lda #-8
	jsr std.print_s8_dec

	lda #petscii.space
	jsr kernel.chrout

	;	..........................	print_u16_dec

	load_imm_ay 1818
	jsr std.print_u16_dec

	lda #petscii.space
	jsr kernel.chrout

	;	..........................	print_s16_dec

	load_imm_ay -1818
	jsr std.print_s16_dec

	lda #petscii.space
	jsr kernel.chrout
	
	;	..........................	print_string

	load_address_ay message
	jsr std.print_string_basic

	lda #petscii.space
	jsr kernel.chrout
	
	;	..........................	print_string_check

	load_address_ay messageErr
	jsr std.print_string

	lda #petscii.space
	jsr kernel.chrout

	;

	rts
	
	;
	
.endproc



