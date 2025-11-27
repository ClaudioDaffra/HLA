#line 1 "/home/claudio/HLA/tst/test9.hla"



CG0 : 0.0+0.0 ;

sys prova (real:fac2,real:r10,byte:hla_reg9,byte:x,word:ay )
{
// asm { ... }  ;


#line 9 "/home/claudio/HLA/tst/test9.hla"
} ;



fn main ( )
{
    au8		:	u8 ;
	bs8  	:	s8 ;
    au16	:	u16 ;
	bs16  	:	s16 ;
	f		:	u8 ;
	fx		:	f40;
	CL34	:	34 ;
	DFF		:	"daffra";
	ps		:	^u8 ;
	LF40	:	0.0 ;


// loop



#line 29 "/home/claudio/HLA/tst/test9.hla"
	au8 := 0 ;
/*		
	loop (au8<10) {
	
		asm {
			lda #'*'
			jsr $ffd2
		};
	
		++au8;
	} ;
*/

/*
	loop  {
	
		asm {
			lda #'*'
			jsr $ffd2
		};
	
		++au8;
	} (au8<10) ;
*/


/*
	loop (au8:=0;au8<10;au8++) {
		asm {
			lda #'*'
			jsr $ffd2
		};
	} ;

	loop {
		break ;
	} ;
*/

// 0 <= 5

#line 65 "/home/claudio/HLA/tst/test9.hla"
	if ( au8 <= 5 )
	{
		asm {
			lda #'#'
			jsr $ffd2
		};
	} ;

	ret 0 ;
} ;


