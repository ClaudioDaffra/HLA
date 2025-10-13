
; High Level Assembler
; Claudio Daffra [2025]

.include "../nMOSLib/lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../nMOSLib/lib/libCD_c64.asm"

; --- Global Variable Declarations ---

; (No global variables)

; --- Code Section ---

stdio .namespace

putChar .proc

    pla
    sta retA
    pla
    sta retY

	; -------------------------- BEGIN

    .fstack_pop_byte_fast

    jsr kernel.chrout


	; -------------------------- END

    lda retY
    pha
    lda retA
    pha

    rts

 retY .byte 0
 retA .byte 0

.endproc

.endnamespace ; stdio

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

	; --- SysbCall to stdio.putChar ---

	; --- BEGIN SYS param push ---

    lda #72

    .fstack_push_byte_fast

	; --- END SYS param push ---

    jsr stdio.putChar
	
	; --- SysbCall to stdio.putChar ---

	; --- BEGIN SYS param push ---

    lda #76

    .fstack_push_byte_fast

	; --- END SYS param push ---

    jsr stdio.putChar
	
	; --- SysbCall to stdio.putChar ---

	; --- BEGIN SYS param push ---

    lda #65

    .fstack_push_byte_fast

	; --- END SYS param push ---

    jsr stdio.putChar
	
	; return
    lda #0


	; -------------------------- END

	; ripristina bp e sp
	.fstack_pop_sp_bp

    rts

.endproc

hla_heap:

;;;
;;
;
