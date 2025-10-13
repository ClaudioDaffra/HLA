
; High Level Assembler
; Claudio Daffra [2025]

.include "../nMOSLib/lib/libCD.asm"

PROGRAM TARGET_C64, PROGRAM_ADDRESS_C64 , 2025

.include "../nMOSLib/lib/libCD_c64.asm"

; --- Global Variable Declarations ---

a          	.byte 0
pa         	.word 0
b          	.word 0
pb         	.word 0
r          	.byte 0,0,0,0,0
pr         	.word 0
a2         	.byte 0
b2         	.word 0
r2         	.byte 0,0,0,0,0

; --- Code Section ---

main .proc

	; --- Stack Frame for function main ---
	fstack_local .struct
		argc           .byte 0
		argv           .word 0
		l1             .byte 0
		pl1            .word 0
		l12            .byte 0
		lw1            .word 0
		plw1           .word 0
		lw12           .word 0
		lr1            .byte 0,0,0,0,0
		plr1           .word 0
		lr12           .byte 0,0,0,0,0
	.endstruct

	.cerror(size(fstack_local) > 255)	; check fstack size

	.weak	; offset names
		argc           = (size(fstack_local)-(fstack_local.argc))
		argv           = (size(fstack_local)-(fstack_local.argv))
		l1             = (size(fstack_local)-(fstack_local.l1))
		pl1            = (size(fstack_local)-(fstack_local.pl1))
		l12            = (size(fstack_local)-(fstack_local.l12))
		lw1            = (size(fstack_local)-(fstack_local.lw1))
		plw1           = (size(fstack_local)-(fstack_local.plw1))
		lw12           = (size(fstack_local)-(fstack_local.lw12))
		lr1            = (size(fstack_local)-(fstack_local.lr1))
		plr1           = (size(fstack_local)-(fstack_local.plr1))
		lr12           = (size(fstack_local)-(fstack_local.lr12))
	.endweak

	; salva bp e sp
	.fstack_push_sp_bp

	; alloca le dimensioni dello stack
	lda #size(fstack_local)
	jsr fstack.alloc

	; -------------------------- BEGIN

	; get address Global a @
    lda #<a
    ldy #>a

    sta pa+0
    sty pa+1

    lda #123

	; Dereference pointer (set value)
    pha

    lda pa+0
    ldy pa+1

    sta zpWord0+0
    sty zpWord0+1

    pla
    .mem_store_byte_pointer

	; Dereference pointer (get value)
    lda pa+0
    ldy pa+1

    .mem_load_byte_pointer

    sta a2

    lda a2
    jsr std.print_u8_dec

    .std_print_nl

	; get address Global b @
    lda #<b
    ldy #>b

    sta pb+0
    sty pb+1

    lda #<456
    ldy #>456

	; Dereference pointer (set value)
    pha
    tya
    pha

    lda pb+0
    ldy pb+1

    sta zpWord0+0
    sty zpWord0+1

    pla
    tay
    pla
    .mem_store_word_pointer

	; Dereference pointer (get value)
    lda pb+0
    ldy pb+1

    .mem_load_word_pointer

    sta b2+0
    sty b2+1

    lda b2+0
    ldy b2+1
    jsr std.print_u16_dec

    .std_print_nl

	; get address Global r @
    lda #<r
    ldy #>r

    sta pr+0
    sty pr+1

	; Load real constant 1024.768 into fac1
    lda #<kConst.Real000
    ldy #>kConst.Real000
    jsr basic.load5_fac1

	; Dereference pointer (set value)
    jsr fstack.push.fac1_c64

    lda pr+0
    ldy pr+1

    sta zpWord0+0
    sty zpWord0+1

    jsr fstack.pop.fac1_c64
    .mem_store_real_pointer

	; Dereference pointer (get value)
    lda pr+0
    ldy pr+1

    .mem_load_real_pointer

	; Set global real 'r2' from fac1
    lda #<r2
    ldy #>r2
    jsr basic.store_fac1

    lda #<r2
    ldy #>r2
    jsr basic.load5_fac1
    jsr std.print_fac1

    .std_print_nl

	; get address Local l1 @
    lda #l1
    jsr fstack.lea_bp

    .fstack_word_set pl1

    lda #98

	; Dereference pointer (set value)
    pha

    .fstack_word_get pl1

    sta zpWord0+0
    sty zpWord0+1

    pla
    .mem_store_byte_pointer

	; Dereference pointer (get value)
    .fstack_word_get pl1

    .mem_load_byte_pointer

    .fstack_byte_set l12

    .fstack_byte_get l12
    jsr std.print_u8_dec

    .std_print_nl

	; get address Local lw1 @
    lda #lw1
    jsr fstack.lea_bp

    .fstack_word_set plw1

    lda #<987
    ldy #>987

	; Dereference pointer (set value)
    pha
    tya
    pha

    .fstack_word_get plw1

    sta zpWord0+0
    sty zpWord0+1

    pla
    tay
    pla
    .mem_store_word_pointer

	; Dereference pointer (get value)
    .fstack_word_get plw1

    .mem_load_word_pointer

    .fstack_word_set lw12

    .fstack_word_get lw12
    jsr std.print_u16_dec

    .std_print_nl

	; get address Local lr1 @
    lda #lr1
    jsr fstack.lea_bp

    .fstack_word_set plr1

	; Load real constant 3.2 into fac1
    lda #<kConst.Real001
    ldy #>kConst.Real001
    jsr basic.load5_fac1

	; Dereference pointer (set value)
    jsr fstack.push.fac1_c64

    .fstack_word_get plr1

    sta zpWord0+0
    sty zpWord0+1

    jsr fstack.pop.fac1_c64
    .mem_store_real_pointer

	; Dereference pointer (get value)
    .fstack_word_get plr1

    .mem_load_real_pointer

    .fstack_fac1_set lr12

    .fstack_fac1_get lr12
    jsr std.print_fac1

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
	Real001   	.byte $82,$4C,$CC,$CC,$CC      	;          3.2 	[130  76 204 204 204]

.endnamespace

hla_heap
;;;
;;
;
