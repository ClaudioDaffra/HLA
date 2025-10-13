
; High Level Assembler
; Claudio Daffra [2025]

.include "../nMOSLib/lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../nMOSLib/lib/libCD_c64.asm"

; --- Global Variable Declarations ---

varu16     	.word 0

; --- Code Section ---

standard .namespace

print .proc

	; salva bp e sp
	.fstack_push_sp_bp

	; -------------------------- BEGIN

	; return
    lda #0


	; -------------------------- END

	; ripristina bp e sp
	.fstack_pop_sp_bp

    rts

.endproc

.endnamespace ; standard

main .proc

	; --- Stack Frame for function main ---
	fstack_local .struct
		argc           .byte 0
		argv           .word 0
		vu8            .byte 0
		vu16           .word 0
		vf40           .byte 0,0,0,0,0
		pu8            .word 0
		pu16           .word 0
		pf40           .word 0
		addr           .word 0
		pString        .word 0
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		argc           = (size(fstack_local)-(fstack_local.argc))
		argv           = (size(fstack_local)-(fstack_local.argv))
		vu8            = (size(fstack_local)-(fstack_local.vu8))
		vu16           = (size(fstack_local)-(fstack_local.vu16))
		vf40           = (size(fstack_local)-(fstack_local.vf40))
		pu8            = (size(fstack_local)-(fstack_local.pu8))
		pu16           = (size(fstack_local)-(fstack_local.pu16))
		pf40           = (size(fstack_local)-(fstack_local.pf40))
		addr           = (size(fstack_local)-(fstack_local.addr))
		pString        = (size(fstack_local)-(fstack_local.pString))
	.endweak

	; salva bp e sp
	.fstack_push_sp_bp

	; alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	; -------------------------- BEGIN

	; Load address of string literal
    lda #<kConst.String000
    ldy #>kConst.String000

    .fstack_word_set pString

    .std_print_nl

	; get address Local vu8 @
    lda #vu8
    jsr fstack.lea_bp

    .fstack_word_set pu8

    .fstack_word_get pu8
    jsr std.print_u16_dec

    .std_print_nl

	; get address Local vu16 @
    lda #vu16
    jsr fstack.lea_bp

    .fstack_word_set pu16

    .fstack_word_get pu16
    jsr std.print_u16_dec

    .std_print_nl

	; get address Local vf40 @
    lda #vf40
    jsr fstack.lea_bp

    .fstack_word_set pf40

    .fstack_word_get pf40
    jsr std.print_u16_dec

    .std_print_nl

    .fstack_word_get pu8

    pha
    tya
    pha

    lda #1

    .cast_from_u8_to_u16

    sty zpWord0+1
    sta zpWord0+0
    pla
    tay
    pla

    jsr math.add_u16

    .fstack_word_set pu8

    .fstack_word_get pu8
    jsr std.print_u16_dec

    .std_print_nl

    .fstack_word_get pu16

    pha
    tya
    pha

    lda #1

    .cast_from_u8_to_u16

    pha
    tya
    pha

    lda #<2
    ldy #>2

    sty zpWord0+1
    sta zpWord0+0
    pla
    tay
    pla

    jsr math.mul_u16

    sty zpWord0+1
    sta zpWord0+0
    pla
    tay
    pla

    jsr math.add_u16

    .fstack_word_set pu16

    .fstack_word_get pu16
    jsr std.print_u16_dec

    .std_print_nl

    .fstack_word_get pf40

    pha
    tya
    pha

    lda #1

    .cast_from_u8_to_u16

    pha
    tya
    pha

    lda #<5
    ldy #>5

    sty zpWord0+1
    sta zpWord0+0
    pla
    tay
    pla

    jsr math.mul_u16

    sty zpWord0+1
    sta zpWord0+0
    pla
    tay
    pla

    jsr math.add_u16

    .fstack_word_set pf40

    .fstack_word_get pf40
    jsr std.print_u16_dec

	; Load address of string literal
    lda #<kConst.String001
    ldy #>kConst.String001

    .fstack_word_set pString

    .fstack_word_get pString
    jsr std.print_string

	; --- Function to standard.print ---

    jsr standard.print
	
	; return
    lda #0


	; -------------------------- END

	; ripristina bp e sp
	.fstack_pop_sp_bp

    rts

.endproc

kConst .namespace

	; --- Locals constants ---

	String000 	.null "claudio daffra"
	String001 	.null "claudio daffra"

.endnamespace

hla_heap
;;;
;;
;
