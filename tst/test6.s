
; High Level Assembler
; Claudio Daffra [2025]

.include "../nMOSLib/lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../nMOSLib/lib/libCD_c64.asm"

; --- Global Variable Declarations ---

; (No global variables)

; --- Code Section ---

standard .namespace

show .proc

	; --- Stack Frame for function standard.show ---
	fstack_local .struct
		b              .byte 0
		w              .word 0
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		b              = (size(fstack_local)-(fstack_local.b))
		w              = (size(fstack_local)-(fstack_local.w))
	.endweak

	; salva bp e sp
	.fstack_push_sp_bp

	; alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	; -------------------------- BEGIN

    std_print_nl
    .fstack_word_get w
    jsr std.print_u16_dec

    std_print_nl
    .fstack_byte_get b
    jsr std.print_u8_dec

	; return
    lda #0


	; -------------------------- END

	; ripristina bp e sp
	.fstack_pop_sp_bp

    rts

.endproc

.endnamespace ; standard

system .namespace

poke .proc

    pla
    sta retA
    pla
    sta retY

	; -------------------------- BEGIN

    .fstack_pop_word_fast
    sty zpWord0+1
    sta zpWord0+0

    .fstack_pop_byte_fast

    ldy #0
    sta (zpWord0),y

    lda #0


	; -------------------------- END

    lda retY
    pha
    lda retA
    pha

    rts

 retY .byte 0
 retA .byte 0

.endproc

.endnamespace ; system

main .proc

	; --- Stack Frame for function main ---
	fstack_local .struct
		argc           .byte 0
		argv           .word 0
		f1             .byte 0,0,0,0,0
		f2             .byte 0,0,0,0,0
		u81            .byte 0
		u161           .word 0
		s161           .word 0
		vbyte          .byte 0
		vbyte2         .byte 0
		preal          .word 0
		s81            .byte 0
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		argc           = (size(fstack_local)-(fstack_local.argc))
		argv           = (size(fstack_local)-(fstack_local.argv))
		f1             = (size(fstack_local)-(fstack_local.f1))
		f2             = (size(fstack_local)-(fstack_local.f2))
		u81            = (size(fstack_local)-(fstack_local.u81))
		u161           = (size(fstack_local)-(fstack_local.u161))
		s161           = (size(fstack_local)-(fstack_local.s161))
		vbyte          = (size(fstack_local)-(fstack_local.vbyte))
		vbyte2         = (size(fstack_local)-(fstack_local.vbyte2))
		preal          = (size(fstack_local)-(fstack_local.preal))
		s81            = (size(fstack_local)-(fstack_local.s81))
	.endweak

	; salva bp e sp
	.fstack_push_sp_bp

	; alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	; -------------------------- BEGIN

    lda #-128

    .cast_from_s8_to_s16

    .fstack_word_set s161

	; --- postfix -- ---
    .fstack_word_get s161

    tax	;	save    AY
    pha
    tya
    pha
    txa	;	restore AY

    dec_word_ay

	; Store result back to variable 's161'
    .fstack_word_set s161

	; Restore original value for postfix result
    pla
    tay
    pla
	; --- end postfix -- ---

    .fstack_word_get s161
    jsr std.print_s16_dec

    lda #-128

    .cast_from_s8_to_s16

    .fstack_word_set s161

	; --- postfix ++ ---
    .fstack_word_get s161

    tax	;	save    AY
    pha
    tya
    pha
    txa	;	restore AY

    inc_word_ay

	; Store result back to variable 's161'
    .fstack_word_set s161

	; Restore original value for postfix result
    pla
    tay
    pla
	; --- end postfix ++ ---

    .std_print_nl
    .fstack_word_get s161
    jsr std.print_s16_dec

	; return
    lda #0


	; -------------------------- END

	; ripristina bp e sp
	.fstack_pop_sp_bp

    rts

.endproc

hla_heap
;;;
;;
;
