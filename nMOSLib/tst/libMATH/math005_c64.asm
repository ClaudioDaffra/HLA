

; .............. commodore 64

; .............. compile : sh casm.sh tst/libMATH/math005_c64

.include "../../lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../../lib/libCD_c64.asm"

kLocalAsmReal00                     .byte  $81,$33,$33,$33,$33	;	// 1.4
kLocalAsmReal01                     .byte  $82,$4c,$cc,$cc,$cc	;   // 3.2
kLocalAsmReal03                   	.byte  $8b,$00,$18,$93,$74 	;  //  1024.768
kLocalAsmReal04                   	.byte  $8b,$80,$18,$93,$74 	;  // -1024.768
message								.null  "the end"


debugA .macro
	ldx #\1
	sta 1024,x
.endmacro


;	101010 011010 011 101101 10
;   001001 1001

main .proc

	lda #147
	jsr kernel.chrout

	;	-------------------------------------- s8 ##	2 ?	=	+2	->	1

	lda #+2	;rhs				
	sta zpWord0+0
	lda #+2	;lhs
	jsr math.s8_cmp_eq
	debugA 0
	
	;	-------------------------------------- u8 ##	1	?=	2	->	0
	
	lda #2	;rhs				
	sta zpWord0+0
	lda #1	;lhs
	jsr math.u8_cmp_eq
	debugA 1

	;	...................................... u16 ##	512 ?= 1024	->	0
	
	lda #<1024	
	ldy #>1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<512
	ldy #>512
	jsr math.u16_cmp_eq
	debugA 2

	;	...................................... s16 ##	512 ?= -1024	->	0	
	
	lda #<-1024
	ldy #>-1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<512
	ldy #>512
	jsr math.s16_cmp_eq
	debugA 3

	;	-------------------------------------- f40 ##	1.4 ?=	3.2 	->	0
	; 1.4
	#load_fac1 kLocalAsmReal00
	; 3.2
	#load_fac2 kLocalAsmReal01
	jsr math.f40_cmp_eq
	debugA 4

	;	--------------------------------------  !F40 ##	!1.4 = 0
	; 1.4
	#load_fac1 kLocalAsmReal00
	jsr math.not_f40
	debugA 40	
	;	--------------------------------------  !F40 ##	!0.0 = 1 
	; 0.0
	#load_fac1 math.zero
	jsr math.not_f40
	debugA 41	
	;	-------------------------------------- 
	
	lda #13
	jsr kernel.chrout
	
	rts
	
.endproc


.comment

 operatori relazionali esconono sempre 		esce con A==0 A==1
	if (a<0) { } else { } ;
 esspressioni inject == operator allora		esce con A==0 A==1
	if (a) { } else { } ;
	if (a==0) { } else { } ;
	if (ay==00) { } else { } ;
	if (fac1==0.0) { } else { } ;
	
	; -------------------------------------
	if000:
		(expr with relop)	a1 a0
		bne esle000:
		...
		jmp endif000:
		else000:
		...
	endif000:
	
	; -------------------------------------
	if000:
		(expr with relop)	a1 a0
		bne endif000:
		...
	endif000:
	
	; -------------------------------------
	
	...jsr math.not_u8
	...jsr math.not_s8
	jsr math.not_u16
	jsr math.not_s16
	jsr math.not_f40
	
.endc
