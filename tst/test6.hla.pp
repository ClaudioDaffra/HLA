#line 1 "/home/claudio/HLA/tst/test6.hla"

ns standard
{
	fn  show ( b:u8,w:u16) -> u8
	{
		asm {
			std_print_nl
			.fstack_word_get w
			jsr std.print_u16_dec
		} ;
		asm {
			std_print_nl
			.fstack_byte_get b
			jsr std.print_u8_dec
		} ;
		ret 0 ;
	} ;

} ;

ns system
{
	sys poke ( word:zpWord0,byte:a ) -> u8
	{
		asm {
			ldy #0
			sta (zpWord0),y
		} ;
// return unsigned char u8

#line 29 "/home/claudio/HLA/tst/test6.hla"
		asm {
			lda #0
		} ;
	} ;
} ;


fn main ( )
{
	f1		:	f40 ;
	f2 	 	: 	f40 ;
	u81 	:   u8	;
	u161 	:   u16	;
	s161 	:	s16 ;
	vbyte 	:	u8	;
	vbyte2 	:	u8	;
	preal 	:	^f40;
	s81 	:	s8 	;
/*	
	// ---------------------- < > <= >=
	
	f1:=1.2;
	f2:=3.2;
	
	u161 := f1 < f2  ;
	
	asm {
		.fstack_word_get u161
		jsr std.print_u16_dec
	} ;

	u161 := 1.2 > 3.4  ;
	
	asm {
		.fstack_word_get u161
		jsr std.print_u16_dec
	} ;
*/

/*	
	// ---------------------- ++ --
	
	vbyte := 7 ;
	vbyte2 := ++vbyte ;

	asm {
		.std_print_nl
		.fstack_byte_get vbyte2
		jsr std.print_u8_dec
	} ;

	vbyte := 7 ;
	++vbyte ;

	asm {
		.std_print_nl
		.fstack_byte_get vbyte
		jsr std.print_u8_dec
	} ;

	vbyte := 7 ;
	vbyte-- ;

	asm {
		.std_print_nl
		.fstack_byte_get vbyte
		jsr std.print_u8_dec
	} ;
*/

//	---------------------------------------- pointer real ++

/*	
	preal := @f1 ;

	asm {
		.std_print_nl
		.fstack_word_get preal
		jsr std.print_u16_dec
	} ;

	++preal ;
	--preal ;
	
	asm {
		.std_print_nl
		.fstack_word_get preal
		jsr std.print_u16_dec
	} ;
*/

//	---------------------------------------- signed 8 -- ++	[ok]

/*
	s81 := -128;
	s81-- ; 			// 127
	
	asm {
		.std_print_nl
		.fstack_byte_get s81
		jsr std.print_s8_dec
	} ;

	s81 := -128;
	s81++ ; 		// -127
	
	asm {
		.std_print_nl
		.fstack_byte_get s81
		jsr std.print_s8_dec
	} ;

	s81 := -128;
	--s81 ; 			// 127
	
	asm {
		.std_print_nl
		.fstack_byte_get s81
		jsr std.print_s8_dec
	} ;

	s81 := -128;
	++s81; 		// -127
	
	asm {
		.std_print_nl
		.fstack_byte_get s81
		jsr std.print_s8_dec
	} ;
*/


//	---------------------------------------- signed word ++ --

/*
		s161 := -128;
		
		--s161 ;					//	-129
		
		asm {
			.std_print_nl
			.fstack_word_get s161
			jsr std.print_s16_dec
		} ;
	
		s161 := -128;				//	-127
		++s161 ;
		
		asm {
			.std_print_nl
			.fstack_word_get s161
			jsr std.print_s16_dec
		} ;
*/


#line 176 "/home/claudio/HLA/tst/test6.hla"
		s161 := -128;

		s161-- ;

		asm {
			.fstack_word_get s161
			jsr std.print_s16_dec
		} ;

		s161 := -128;
		s161++ ;

		asm {
			.std_print_nl
			.fstack_word_get s161
			jsr std.print_s16_dec
		} ;

/*
	u161 := show ;
	
	u161 := standard::show( 2,53281 ) ; // 1 , $d021 209 32
	
	u161 := system::poke( 53280,1 ) ; // 1 , $d020 208 32
*/


#line 201 "/home/claudio/HLA/tst/test6.hla"
	ret 0 ;
} ;


