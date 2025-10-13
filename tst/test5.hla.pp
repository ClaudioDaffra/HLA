#line 1 "/home/claudio/HLA/tst/test5.hla"

// PASSAGGIO STANDARD HLA, registri alla fine


//	a y x 			->  registri

//	fac1 , fac2		->	registri


// nelle sys non c'è return !



#line 9 "/home/claudio/HLA/tst/test5.hla"
sys uno(  word:zpWord0,byte:a )
{
	asm {
		; body
	} ;
} ;

// nelle fn c'è return !



#line 18 "/home/claudio/HLA/tst/test5.hla"
fn due(  x:u16,y:u8 ) -> u8
{

	ret 0 ;
} ;

// nelle sys c'è sempre asm { ... } ;


#line 25 "/home/claudio/HLA/tst/test5.hla"
sys putchar()
{
	asm {
		jsr kernel.chrout
	} ;
} ;

// PASSAGGIO NON STANDARD HLA, CODIFICA MANUALE ATTENZIONE !!!



#line 34 "/home/claudio/HLA/tst/test5.hla"
asm {

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

} ;

fn main ( )
{

	asm {

		lda #1
		pha

		lda #<512
		ldy #>512
		pha
		tya
		pha

		jsr sys_uno
	} ;

	ret 0 ;
} ;


