
; High Level Assembler
; Claudio Daffra [2025]

.include "../nMOSLib/lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../nMOSLib/lib/libCD_c64.asm"

; --- Global Variable Declarations ---

; (No global variables)

; --- Code Section ---

uno .proc

    pla
    sta retA
    pla
    sta retY

	; -------------------------- BEGIN

    .fstack_pop_word_fast
    sty zpWord0+1
    sta zpWord0+0

    .fstack_pop_byte_fast

    ; body


	; -------------------------- END

    lda retY
    pha
    lda retA
    pha

    rts

 retY .byte 0
 retA .byte 0

.endproc

due .proc

	; --- Stack Frame for function due ---
	fstack_local .struct
		x              .word 0
		y              .byte 0
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		x              = (size(fstack_local)-(fstack_local.x))
		y              = (size(fstack_local)-(fstack_local.y))
	.endweak

	; salva bp e sp
	.fstack_push_sp_bp

	; alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	; -------------------------- BEGIN

	; return
    lda #0


	; -------------------------- END

	; ripristina bp e sp
	.fstack_pop_sp_bp

    rts

.endproc

putchar .proc

	; -------------------------- BEGIN

    jsr kernel.chrout


	; -------------------------- END

    rts

.endproc
    sys_uno .proc	; (  a , y , x , zpWord0:u16 , fac1 , fac2 )
    ; 	save ret address
    pla
    sta ret_lo
    pla
    sta ret_hi
    ; param pop
    ; .....................
    pla
    sta $6e	;	110
    pla
    sta $6d	;	109
    pla
    sta $6c	;	108
    pla
    sta $6b	;	107
    pla
    sta $6a	;	106
    pla
    sta $69	;	105
    ; .....................
    pla
    sta $66
    pla
    sta $65
    pla
    sta $64
    pla
    sta $63
    pla
    sta $62
    pla
    sta $61
    ; .....................
    pla
    sta zpWord0+0
    pla
    sta zpWord0+1
    pla
    ; .....................	param begin
    pla
    tax
    pla
    tay
    pla
    ; .....................	param end
    ; ---------------- BEGIN
    ; body
    ; ---------------- END
    ; 	restore ret address
    lda ret_hi
    pha
    lda ret_lo
    pha
    rts
    ret_lo .byte 0
    ret_hi .byte 0
    .endproc
    ; ULTERIORE ESEMPIO : PASSAGGIO NON STANDARD HLA, CODIFICA MANUALE ATTENZIONE !!!
    ln_uno .macro	; (  a:byte , zpWord0:word )
    ; ---------------- param
    pla
    sta zpWord0+0
    pla
    sta zpWord0+1
    pla
    ; ---------------- BEGIN
    ;	body
    ; ---------------- END
    .endmacro


main .proc

	; --- Stack Frame for function main ---
	fstack_local .struct
		argc           .byte 0
		argv           .word 0
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		argc           = (size(fstack_local)-(fstack_local.argc))
		argv           = (size(fstack_local)-(fstack_local.argv))
	.endweak

	; salva bp e sp
	.fstack_push_sp_bp

	; alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	; -------------------------- BEGIN

    lda #1
    pha
    lda #<512
    ldy #>512
    pha
    tya
    pha
    jsr sys_uno

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
