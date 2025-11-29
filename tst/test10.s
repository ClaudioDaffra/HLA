
; High Level Assembler
; Claudio Daffra [2025]

.include "../nMOSLib/lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../nMOSLib/lib/libCD_c64.asm"

; --- Global Variable Declarations ---

garr1      	.fill 10 	; u8[10] (10*1 = 10)
garr2      	.fill 320 	; f40[8][8] (8*8*5 = 320)
garr3      	.fill 20 	; *s16[10] (10*2 = 20)

; --- Code Section ---

main .proc

	; --- Stack Frame for function main ---
	fstack_local .struct
		argc           .byte 0
		argv           .word 0
		larr1          .fill 10 ; u8[10] (10*1 = 10)
		larr2          .fill 45 ; f40[3][3] (3*3*5 = 45)
		larr3          .fill 20 ; *s16[10] (10*2 = 20)
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		argc           = (size(fstack_local)-(fstack_local.argc))
		argv           = (size(fstack_local)-(fstack_local.argv))
		larr1          = (size(fstack_local)-(fstack_local.larr1))
		larr2          = (size(fstack_local)-(fstack_local.larr2))
		larr3          = (size(fstack_local)-(fstack_local.larr3))
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

hla_heap:

;;;
;;
;
