
; High Level Assembler
; Claudio Daffra [2025]

.include "../nMOSLib/lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../nMOSLib/lib/libCD_c64.asm"

; --- Global Variable Declarations ---

aaa        	.word 0

; --- Code Section ---

sub1 .proc

	; --- Stack Frame for function sub1 ---
	fstack_local .struct
		x              .byte 0
		y              .word 0
		p1             .word 0
		p2             .word 0
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		x              = (size(fstack_local)-(fstack_local.x))
		y              = (size(fstack_local)-(fstack_local.y))
		p1             = (size(fstack_local)-(fstack_local.p1))
		p2             = (size(fstack_local)-(fstack_local.p2))
	.endweak

	; salva bp e sp
	.fstack_push_sp_bp

	; alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	; -------------------------- BEGIN


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
		m1             .word 0
		m2             .word 0
		vs16           .word 0
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		argc           = (size(fstack_local)-(fstack_local.argc))
		argv           = (size(fstack_local)-(fstack_local.argv))
		m1             = (size(fstack_local)-(fstack_local.m1))
		m2             = (size(fstack_local)-(fstack_local.m2))
		vs16           = (size(fstack_local)-(fstack_local.vs16))
	.endweak

	; salva bp e sp
	.fstack_push_sp_bp

	; alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	; -------------------------- BEGIN

    lda #1

    .cast_from_u8_to_u16

    .fstack_word_set m1

    lda #0

    .cast_from_u8_to_u16

    sta aaa+0
    sty aaa+1

    lda #124

    pha

    lda #5

    jsr math.not_u8

    sta zpWord0+0
    pla

    sec
    sbc zpWord0+0

    pha

    lda #3

    sta zpWord0+0
    pla

    ldy zpWord0+0
    jsr math.mod_u8

    .cast_from_u8_to_u16

    pha
    tya
    pha

    lda aaa+0
    ldy aaa+1

    sty zpWord0+1
    sta zpWord0+0
    pla
    tay
    pla

    jsr math.sub_u16

    jsr math.neg_u16

    .cast_from_u16_to_s16

    .fstack_word_set vs16

    .fstack_word_get vs16
    jsr std.print_s16_dec
    lda #petscii.nl
    jsr kernel.chrout

    lda #<-1499
    ldy #>-1499

    .fstack_word_set vs16

    .fstack_word_get vs16
    jsr std.print_s16_dec
    lda #petscii.nl
    jsr kernel.chrout

    lda #<-213
    ldy #>-213

    .fstack_word_set vs16

    .fstack_word_get vs16
    jsr std.print_s16_dec
    lda #petscii.nl
    jsr kernel.chrout

    lda #<1323
    ldy #>1323

    .cast_from_u16_to_s16

    .fstack_word_set vs16

    .fstack_word_get vs16
    jsr std.print_s16_dec
    lda #petscii.nl
    jsr kernel.chrout

    lda #125

    .cast_from_u8_to_u16

    sta aaa+0
    sty aaa+1

    jsr std.print_s16_dec
    lda #petscii.nl
    jsr kernel.chrout


	; -------------------------- END

	; ripristina bp e sp
	.fstack_pop_sp_bp

    rts

.endproc

hla_heap
;;;
;;
;
