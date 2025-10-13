
; .............. compile : sh casm.sh tst/libBASIC/basic000_vic20

.include "../../lib/libCD.asm"

PROGRAM TARGET_VIC20, PROGRAM_ADDRESS_VIC20 , 2025

.include "../../lib/libCD_vic20.asm"

kLocalAsmReal00                     .byte  $81,$33,$33,$33,$33	;	// 1.4
kLocalAsmReal01                     .byte  $82,$4c,$cc,$cc,$cc	;   // 3.2
kLocalAsmReal03                   	.byte  $8b,$00,$18,$93,$74 	;  //  1024.768
kLocalAsmReal04                   	.byte  $8b,$80,$18,$93,$74 	;  // -1024.768
message								.null	"the end"

main .proc

	;	print unsigned integer
	load_imm_ay	123
	jsr basic.print_unsigned_integer
	
	lda #petscii.nl
	jsr kernel.chrout

	;	print unsigned integer
	lda #123
	jsr basic.print_unsigned_char

	lda #petscii.space
	jsr kernel.chrout
	
	;	print unsigned integer
	load_imm_ay 456
	jsr basic.print_unsigned_integer

	;	u8 to fac1
	lda #petscii.space
	jsr kernel.chrout
	
	lda #8
	jsr basic.conv_u8_to_fac1
	jsr basic.print_fac1
	
	;	s8 to fac1
	lda #petscii.space
	jsr kernel.chrout
	
	lda #-8
	jsr basic.conv_s8_to_fac1
	jsr basic.print_fac1

	;	conv s16 u16 to fac1
	
	lda #petscii.nl
	jsr kernel.chrout
	
	load_imm_ay	-768
	jsr basic.conv_s16_to_fac1
	jsr basic.print_fac1

	load_imm_ay	768
	jsr basic.conv_u16_to_fac1
	jsr basic.print_fac1

	;	print float

	lda #petscii.nl
	jsr kernel.chrout

	load_fac1 kLocalAsmReal00
	jsr basic.print_fac1

	;	print string
	
	lda #petscii.nl
	jsr kernel.chrout

	load_address_ay	message
	jsr basic.print_string

	;	convert fac1 to DWord ;  -1024.768
	
	load_fac1 kLocalAsmReal04
	jsr basic.conv_fac1_to_DWord0	;	ayxt	-1025
	jsr std.print_s16_dec

	; store fac1 to address

	load_fac1	kLocalAsmReal00
	load_address_ay	250
	jsr basic.store_fac1
	load_fac1	250
	jsr basic.print_fac1

	; store fac1 to address

	lda #petscii.nl
	jsr kernel.chrout
	
	load_fac1	kLocalAsmReal00
	jsr basic.store_fac1_in_fac2	
	jsr basic.store_fac2_in_fac1
	jsr basic.print_fac1
	
	rts

	;
	
.endproc
