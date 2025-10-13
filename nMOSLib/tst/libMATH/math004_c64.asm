

; .............. commodore 64

; .............. compile : sh casm.sh tst/libMATH/math004_c64

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
debugb .macro
	ldx #\1
	sta 1024+40,x
.endmacro

;	101010 011010 011 101101 10
;   001001 1001

main .proc

	lda #147
	jsr kernel.chrout
.comment	
	;	-------------------------------------- u8 	a	<	zpWord0
	
	lda #1	;rhs		;	2 < 1	->	0
	sta zpWord0+0
	lda #2	;lhs
	jsr math.u8_cmp_lt
	debugA 0

	lda #2				;	1 < 2	->	1
	sta zpWord0+0
	lda #1
	jsr math.u8_cmp_lt
	debugA 1

	;	-------------------------------------- s8 	a	<	zpWord0
	
	lda #-1	;rhs				;	+2 < -1	->	0
	sta zpWord0+0
	lda #+2	;lhs
	jsr math.s8_cmp_lt
	debugA 2

	lda #+2	;rhs				;	-1 < +2	->	1
	sta zpWord0+0
	lda #-1	;lhs
	jsr math.s8_cmp_lt
	debugA 3
	
	lda #-2	;rhs				;	-1 < -2	->	0
	sta zpWord0+0
	lda #-1	;lhs
	jsr math.s8_cmp_lt
	debugA 4

	lda #-1	;rhs				;	-2 < -1 ->	1
	sta zpWord0+0
	lda #-2	;lhs
	jsr math.s8_cmp_lt
	debugA 5

	;	-------------------------------------- u8 	a	<=	zpWord0
	
	lda #1	;rhs				;	2 <= 1	->	0
	sta zpWord0+0
	lda #2	;lhs
	jsr math.u8_cmp_le
	debugA 7

	lda #2	;rhs				;	2 <= 2	->	1
	sta zpWord0+0
	lda #2	;lhs
	jsr math.u8_cmp_le
	debugA 8
	
	lda #2	;rhs				;	1 <= 2	->	1
	sta zpWord0+0
	lda #1	;lhs
	jsr math.u8_cmp_le
	debugA 9

	;	-------------------------------------- s8 	a	<=	zpWord0
	
	lda #-1	;rhs				;	+2 <= -1	->	0
	sta zpWord0+0
	lda #+2	;lhs
	jsr math.s8_cmp_le
	debugA 11

	lda #+2	;rhs				;	-1 <= +2	->	1
	sta zpWord0+0
	lda #-1	;lhs
	jsr math.s8_cmp_le
	debugA 12
	
	lda #-2	;rhs				;	-1 <= -2	->	0
	sta zpWord0+0
	lda #-1	;lhs
	jsr math.s8_cmp_le
	debugA 13

	lda #-1	;rhs				;	-2 <= -1	->	1
	sta zpWord0+0
	lda #-2	;lhs
	jsr math.s8_cmp_le
	debugA 14

	;	-------------------------------------- u8 >
	
	lda #1	;rhs				;	2 > 1	->	1
	sta zpWord0+0
	lda #2	;lhs
	jsr math.u8_cmp_gt
	debugA 16

	lda #2	;rhs				;	1 > 2	->	0
	sta zpWord0+0
	lda #1	;lhs
	jsr math.u8_cmp_gt
	debugA 17

	lda #2	;rhs				;	2 > 2	->	0
	sta zpWord0+0
	lda #2	;lhs
	jsr math.u8_cmp_gt
	debugA 18

	;	-------------------------------------- s8 >
	
	lda #-1	;rhs				;	+2 > -1	->	1
	sta zpWord0+0
	lda #+2	;lhs
	jsr math.s8_cmp_gt
	debugA 19

	lda #+2	;rhs				;	-1 > +2	->	0
	sta zpWord0+0
	lda #-1	;lhs
	jsr math.s8_cmp_gt
	debugA 20

	lda #+2	;rhs				;	+2 > +2	->	0
	sta zpWord0+0
	lda #+2	;lhs
	jsr math.s8_cmp_gt
	debugA 21

	;	-------------------------------------- u8 >=

	lda #1	;rhs				;	2 >= 1		->	1
	sta zpWord0+0
	lda #2	;lhs
	jsr math.u8_cmp_ge
	debugA 23

	lda #2	;rhs				;	1 >= 2		->	0
	sta zpWord0+0
	lda #1	;lhs
	jsr math.u8_cmp_ge
	debugA 24

	lda #2	;rhs				;	2 >= 2		->	1
	sta zpWord0+0
	lda #2	;lhs
	jsr math.u8_cmp_ge
	debugA 25

	;	-------------------------------------- s8 >=

	lda #-1	;rhs				;	+2 >= -1	->	1
	sta zpWord0+0
	lda #+2	;lhs
	jsr math.s8_cmp_ge
	debugA 26

	lda #+2	;rhs				;	-1 >= +2	->	0
	sta zpWord0+0
	lda #-1	;lhs
	jsr math.s8_cmp_ge
	debugA 27

	lda #+2	;rhs				;	+2 >= +2	->	1
	sta zpWord0+0
	lda #+2	;lhs
	jsr math.s8_cmp_ge
	debugA 28
.endc
	;	-------------------------------------- s8 ?=

	lda #+2	;rhs				;	+2 ?= +2	->	1
	sta zpWord0+0
	lda #+2	;lhs
	jsr math.s8_cmp_eq
	debugA 30
	
	;	-------------------------------------- u8 ?=
	
	lda #2	;rhs				;	1 	?= 2	->	0
	sta zpWord0+0
	lda #1	;lhs
	jsr math.u8_cmp_eq
	debugA 31

	;	...................................... u16 ?=		512 ?= 1024		->	0
	
	lda #<1024	
	ldy #>1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<512
	ldy #>512
	jsr math.u16_cmp_eq
	debugA 40

	;	...................................... s16 ?=		512 ?= -1024	->	0	
	
	lda #<-1024
	ldy #>-1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<512
	ldy #>512
	jsr math.s16_cmp_eq
	debugA 41
.comment
	;	...................................... u16 <		
	
	lda #<1024	
	ldy #>1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<512
	ldy #>512
	jsr math.u16_cmp_lt						;		512 < 1024		->	1
	debugA 42

	lda #<512	
	ldy #>512
	sta zpWord0+0
	sty zpWord0+1
	lda #<1024
	ldy #>1024
	jsr math.u16_cmp_lt						;		1024 < 512		->	0
	debugA 43
	
	;	...................................... s16 <
	
	lda #<-1024
	ldy #>-1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<+512
	ldy #>+512
	jsr math.s16_cmp_lt						;		512 < -1024		->	0
	debugA 44

	lda #<+512
	ldy #>+512
	sta zpWord0+0
	sty zpWord0+1
	lda #<-1024
	ldy #>-1024
	jsr math.s16_cmp_lt						;		-1024 < +512	->	1
	debugA 45

	;	...................................... u16 <=		
	
	lda #<1024	
	ldy #>1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<512
	ldy #>512
	jsr math.u16_cmp_le						;		512 <= 1024		->	1
	debugA 47

	lda #<512	
	ldy #>512
	sta zpWord0+0
	sty zpWord0+1
	lda #<1024
	ldy #>1024
	jsr math.u16_cmp_le						;		1024 <= 512		->	0
	debugA 48
	
	;	...................................... s16 <=
	
	lda #<-1024
	ldy #>-1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<+512
	ldy #>+512
	jsr math.s16_cmp_le								;		512 <= -1024	->	0
	debugA 49

	lda #<+512
	ldy #>+512
	sta zpWord0+0
	sty zpWord0+1
	lda #<+512
	ldy #>+512
	jsr math.s16_cmp_le								;		+512 <= +512	->	1
	debugA 50

; ############################
; ############################
; ############################


	;	...................................... s16 >

	lda #<-1024
	ldy #>-1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<+512
	ldy #>+512
	jsr math.s16_cmp_gt								;		+512 > -1024	->	1
	debugA 52

	lda #<+512
	ldy #>+512
	sta zpWord0+0
	sty zpWord0+1
	lda #<+512
	ldy #>+512
	jsr math.s16_cmp_gt								;		+512 > +512		->	0
	debugA 53

	lda #<-1024
	ldy #>-1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<+512
	ldy #>+512
	jsr math.s16_cmp_gt								;		512  > -1024	->	1
	debugA 54

	;	...................................... s16 ?=

	lda #<1024
	ldy #>1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<512
	ldy #>512
	jsr math.u16_cmp_eq								;		512 ?= -1024	->	0
	debugA 57

	lda #<512
	ldy #>512
	sta zpWord0+0
	sty zpWord0+1
	lda #<512
	ldy #>512
	jsr math.u16_cmp_eq								;		+512 ?= +512	->	1
	debugA 58

	lda #<-1024
	ldy #>-1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<+512
	ldy #>+512
	jsr math.s16_cmp_eq								;		512 ?= -1024	->	0
	debugA 59

	lda #<+512
	ldy #>+512
	sta zpWord0+0
	sty zpWord0+1
	lda #<+512
	ldy #>+512
	jsr math.s16_cmp_eq								;		+512 ?= +512	->	1
	debugA 60

	;	...................................... s16 >=

	lda #<-1024
	ldy #>-1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<+512
	ldy #>+512
	jsr math.s16_cmp_ge								;		512 >= -1024	->	1
	debugA 62

	lda #<+512
	ldy #>+512
	sta zpWord0+0
	sty zpWord0+1
	lda #<+512
	ldy #>+512
	jsr math.s16_cmp_ge								;		+512 >= +512	->	1
	debugA 63

	lda #<+1024
	ldy #>+1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<-512
	ldy #>-512
	jsr math.s16_cmp_ge								;		-512 >= +1024	->	0
	debugA 64

	;	...................................... u16 >

	lda #<1024
	ldy #>1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<512
	ldy #>512
	jsr math.u16_cmp_gt								;		512 >  1024	->	0
	debugA 80

	lda #<+512
	ldy #>+512
	sta zpWord0+0
	sty zpWord0+1
	lda #<+512
	ldy #>+512
	jsr math.u16_cmp_gt								;		512 > 512	->	0
	debugA 81

	lda #<512
	ldy #>512
	sta zpWord0+0
	sty zpWord0+1
	lda #<1024
	ldy #>1024
	jsr math.u16_cmp_gt								;		1024 > 512		->	1
	debugA 82

	;	...................................... u16 >

	lda #<1024
	ldy #>1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<512
	ldy #>512
	jsr math.u16_cmp_ge								;		512 >=  1024	->	0
	debugA 84

	lda #<+512
	ldy #>+512
	sta zpWord0+0
	sty zpWord0+1
	lda #<+512
	ldy #>+512
	jsr math.u16_cmp_ge								;		512 >= 512		->	1
	debugA 85

	lda #<512
	ldy #>512
	sta zpWord0+0
	sty zpWord0+1
	lda #<1024
	ldy #>1024
	jsr math.u16_cmp_ge								;		1024 >= 512		->	1
	debugA 86

	;	-------------------------------------- f40 <		1
	; 1.4
	#load_fac1 kLocalAsmReal00
	; 3.2
	#load_fac2 kLocalAsmReal01
	jsr math.f40_cmp_lt
	debugA 120

	;	-------------------------------------- f40 >		0
	; 1.4
	#load_fac1 kLocalAsmReal00
	; 3.2
	#load_fac2 kLocalAsmReal01
	jsr math.f40_cmp_gt
	debugA 121

	;	-------------------------------------- f40 ?=		0
	; 1.4
	#load_fac1 kLocalAsmReal00
	; 3.2
	#load_fac2 kLocalAsmReal01
	jsr math.f40_cmp_eq
	debugA 122

	;	-------------------------------------- f40 <=		1
	; 1.4
	#load_fac1 kLocalAsmReal00
	; 1.4
	#load_fac2 kLocalAsmReal01
	jsr math.f40_cmp_le
	debugA 123

	;	-------------------------------------- f40 >=		0
	; 1.4
	#load_fac1 kLocalAsmReal00
	; 3.2
	#load_fac2 kLocalAsmReal01
	jsr math.f40_cmp_ge
	debugA 125

	; !=
	
	;	-------------------------------------- f40 !=		 	1.4 != 3.2	-> 1
	; 1.4
	#load_fac1 kLocalAsmReal00
	; 3.2
	#load_fac2 kLocalAsmReal01
	jsr math.f40_cmp_ne
	debugA 127

	;	...................................... s16 !=		512 != -1024	->	1	
	
	lda #<-1024
	ldy #>-1024
	sta zpWord0+0
	sty zpWord0+1
	lda #<512
	ldy #>512
	jsr math.s16_cmp_ne
	debugA 128

	;	...................................... u16 !=		512 != -1024	->	0	
	
	lda #<512
	ldy #>512
	sta zpWord0+0
	sty zpWord0+1
	lda #<512
	ldy #>512
	jsr math.u16_cmp_ne
	debugA 129
	
	;	-------------------------------------- !=

	lda #2	;rhs											;	+2 != +2	->	0
	sta zpWord0+0
	lda #2	;lhs
	jsr math.u8_cmp_ne
	debugA 130

	lda #-1	;rhs											;	+2 != -1	->	1
	sta zpWord0+0
	lda #+2	;lhs
	jsr math.s8_cmp_ne
	debugA 131
.endc
	;	-------------------------------------- 
	
	lda #13
	jsr kernel.chrout
	lda #13
	jsr kernel.chrout
	
	rts
	
.endproc

