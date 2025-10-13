
; High Level Assembler
; Claudio Daffra [2025]

.include "../nMOSLib/lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../nMOSLib/lib/libCD_c64.asm"

; --- Global Variable Declarations ---

a          	.word 0
pa         	.word 0

; --- Code Section ---

sub1 .proc

	; --- Stack Frame for function sub1 ---
	fstack_local .struct
		px             .byte 0
		py             .word 0
		pr             .byte 0,0,0,0,0
		p1             .word 0
		p2             .word 0
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		px             = (size(fstack_local)-(fstack_local.px))
		py             = (size(fstack_local)-(fstack_local.py))
		pr             = (size(fstack_local)-(fstack_local.pr))
		p1             = (size(fstack_local)-(fstack_local.p1))
		p2             = (size(fstack_local)-(fstack_local.p2))
	.endweak

	; salva bp e sp
	.fstack_push_sp_bp

	; alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	; -------------------------- BEGIN

	; return
    lda #<1
    ldy #>1

    .cast_from_s16_to_u8


	; -------------------------- END

	; ripristina bp e sp
	.fstack_pop_sp_bp

    rts

.endproc

main .proc

	; --- Stack Frame for function main ---
	fstack_local .struct
		argc           .byte 0
		argv           .word 0
		m1             .byte 0
		m2             .byte 0
		pu8            .word 0
		m3             .byte 0
		m4             .word 0
		r1             .byte 0,0,0,0,0
		r2             .byte 0,0,0,0,0
		r3             .byte 0,0,0,0,0
		x              .byte 0,0,0,0,0
		y              .byte 0,0,0,0,0
		z              .byte 0,0,0,0,0
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		argc           = (size(fstack_local)-(fstack_local.argc))
		argv           = (size(fstack_local)-(fstack_local.argv))
		m1             = (size(fstack_local)-(fstack_local.m1))
		m2             = (size(fstack_local)-(fstack_local.m2))
		pu8            = (size(fstack_local)-(fstack_local.pu8))
		m3             = (size(fstack_local)-(fstack_local.m3))
		m4             = (size(fstack_local)-(fstack_local.m4))
		r1             = (size(fstack_local)-(fstack_local.r1))
		r2             = (size(fstack_local)-(fstack_local.r2))
		r3             = (size(fstack_local)-(fstack_local.r3))
		x              = (size(fstack_local)-(fstack_local.x))
		y              = (size(fstack_local)-(fstack_local.y))
		z              = (size(fstack_local)-(fstack_local.z))
	.endweak

	; salva bp e sp
	.fstack_push_sp_bp

	; alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	; -------------------------- BEGIN

	; --- Function to sub1 ---

	; --- BEGIN FN param push ---

	.fstack_push_sp

    lda #1

    jsr fstack.push.byte

    lda #<128
    ldy #>128

    jsr fstack.push.word

	; Load real constant 1024.768 into fac1
    lda #<kConst.Real000
    ldy #>kConst.Real000
    jsr basic.load5_fac1

    jsr fstack.push.real

	.fstack_pop_sp

	; --- END FN param push ---

    jsr sub1
	
    .cast_from_u8_to_s8

    .fstack_byte_set m1

	; get address Global a @
    lda #<a
    ldy #>a

    sta pa+0
    sty pa+1

    lda #<123
    ldy #>123

	; Dereference pointer (set value)
    pha
    tya
    pha

    lda pa+0
    ldy pa+1

    sta zpWord0+0
    sty zpWord0+1

    pla
    tay
    pla
    .mem_store_word_pointer

    lda a+0
    ldy a+1
    jsr std.print_u16_dec
    lda #petscii.nl
    jsr kernel.chrout

	; return
    lda #0


	; -------------------------- END

	; ripristina bp e sp
	.fstack_pop_sp_bp

    rts

.endproc

kConst .namespace

	; --- Locals constants ---

	Real000   	.byte $8B,$00,$18,$93,$74      	;     1024.768 	[139   0  24 147 116]

.endnamespace

hla_heap
;;;
;;
;
