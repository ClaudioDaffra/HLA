
; High Level Assembler
; Claudio Daffra [2025]

.include "../nMOSLib/lib/libCD.asm"

PROGRAM TARGET_C128, PROGRAM_ADDRESS_C128 , 2025

.include "../nMOSLib/lib/libCD_c128.asm"

; --- Global Variable Declarations ---

; (No global variables)

; --- Code Section ---

prova .proc

    pla
    sta retA
    pla
    sta retY

	; -------------------------- BEGIN

    .fstack_pop_real_fast_in_fac2

    .fstack_pop_real_fast
    lda #<r10
    ldy #>r10
    jsr basic.store_fac1

    .fstack_pop_byte_fast
    sta hla_reg9+0

    .fstack_pop_byte_fast
    tax

    .fstack_pop_word_fast


	; -------------------------- END

    lda retY
    pha
    lda retA
    pha

    rts

 retY .byte 0
 retA .byte 0

.endproc

main .proc

	; --- Stack Frame for function main ---
	fstack_local .struct
		argc           .byte 0
		argv           .word 0
		au8            .byte 0
		bs8            .byte 0
		au16           .word 0
		bs16           .word 0
		f              .byte 0
		fx             .byte 0,0,0,0,0
		ps             .word 0
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		argc           = (size(fstack_local)-(fstack_local.argc))
		argv           = (size(fstack_local)-(fstack_local.argv))
		au8            = (size(fstack_local)-(fstack_local.au8))
		bs8            = (size(fstack_local)-(fstack_local.bs8))
		au16           = (size(fstack_local)-(fstack_local.au16))
		bs16           = (size(fstack_local)-(fstack_local.bs16))
		f              = (size(fstack_local)-(fstack_local.f))
		fx             = (size(fstack_local)-(fstack_local.fx))
		ps             = (size(fstack_local)-(fstack_local.ps))
	.endweak

	; salva bp e sp
	.fstack_push_sp_bp

	; alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	; -------------------------- BEGIN

    .std_print_nl
    .std_print_nl
    .std_print_nl

	; Load global real constant 'CG0' (0) into fac1
    lda #<kConst.CG0
    ldy #>kConst.CG0
    jsr basic.load5_fac1

    .fstack_fac1_set fx

	; Load global real constant 'CG0' (-0) into fac1
    lda #<kConst.CG0
    ldy #>kConst.CG0
    jsr basic.load5_fac1

    .cast_from_f40_to_u8

    pha

    .fstack_fac1_get fx

    .cast_from_f40_to_u8

    sta zpWord0+0
    pla

    jsr math.logical_and_u8s8

    .fstack_byte_set f

    .std_print_nl
    .fstack_byte_get f
    jsr std.print_u8_dec

	; Load real constant 1 into fac1
    lda #<kConst.Real000
    ldy #>kConst.Real000
    jsr basic.load5_fac1

    .cast_from_f40_to_u8

    pha

	; Load global real constant 'CG0' (0) into fac1 ( original LF40 )
    lda #<kConst.CG0	;	<-	#<LF40
    ldy #>kConst.CG0	;	<-	#>LF40
    jsr basic.load5_fac1

    .cast_from_f40_to_u8

    sta zpWord0+0
    pla

    jsr math.logical_and_u8s8

    .fstack_byte_set f

    .std_print_nl
    .fstack_byte_get f
    jsr std.print_u8_dec

	; Load address of string literal
    lda #<kConst.String000
    ldy #>kConst.String000

    .fstack_word_set ps

	; return
    lda #0


	; -------------------------- END

	; ripristina bp e sp
	.fstack_pop_sp_bp

    rts

.endproc

kConst .namespace

	; --- Global constants ---

	CG0       	.byte $00,$00,$00,$00,$00		;      0.00000 	[  0   0   0   0   0]

	; --- Locals constants ---

	Real000   	.byte $81,$00,$00,$00,$00      	;            1 	[129   0   0   0   0]

	String000 	.null "daffra"

.endnamespace

hla_heap:
;;;
;;
;
