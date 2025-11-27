
; High Level Assembler
; Claudio Daffra [2025]

.include "../nMOSLib/lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../nMOSLib/lib/libCD_c64.asm"

; --- Global Variable Declarations ---

; (No global variables)

; --- Code Section ---

main .proc

	; --- Stack Frame for function main ---
	fstack_local .struct
		argc           .byte 0
		argv           .word 0
		i              .byte 0
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		argc           = (size(fstack_local)-(fstack_local.argc))
		argv           = (size(fstack_local)-(fstack_local.argv))
		i              = (size(fstack_local)-(fstack_local.i))
	.endweak

	; salva bp e sp
	.fstack_push_sp_bp

	; alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	; -------------------------- BEGIN

    lda #0

    .fstack_byte_set i

	; loop construction {0}
loop_start_000:
	; loop body
	; --- postfix ++ ---
    .fstack_byte_get i

    pha

    inc_byte_a

	; Store result back to variable 'i'
    .fstack_byte_set i

	; Restore original value for postfix result
    pla
	; --- end postfix ++ ---

	; if {000}
    .fstack_byte_get i

    pha

    lda #5

    sta zpWord0+0
    pla

    jsr math.u8_cmp_gt

	; relational condition
    beq  endif000

	; then {000}
	; break
    jmp loop_end_000

 endif000:
	; end if {000}

loop_step_000:
    jmp loop_start_000
loop_end_000:
	; end loop {0}

    lda #0

    .fstack_byte_set i

	; loop construction {1}
loop_start_001:
	; loop pre-condition check
    .fstack_byte_get i

    pha

    lda #10

    sta zpWord0+0
    pla

    jsr math.u8_cmp_lt

    beq loop_end_001

	; loop body
	; --- postfix ++ ---
    .fstack_byte_get i

    pha

    inc_byte_a

	; Store result back to variable 'i'
    .fstack_byte_set i

	; Restore original value for postfix result
    pla
	; --- end postfix ++ ---

	; if {001}
    .fstack_byte_get i

    pha

    lda #5

    sta zpWord0+0
    pla

    jsr math.u8_cmp_lt

	; relational condition
    beq  endif001

	; then {001}
	; continue
    jmp loop_step_001

 endif001:
	; end if {001}

    lda #'.'
    jsr $ffd2

loop_step_001:
    jmp loop_start_001
loop_end_001:
	; end loop {1}

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
