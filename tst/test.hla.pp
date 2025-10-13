#line 1 "/home/claudio/HLA/tst/test.hla"

	aaa : u16 ;

	fn sub1 ( x:u8,y:u16 ) -> ^u8
	{
		p1 : u16 ;
		p2 : ^u16 ;
	} ;

	fn main ( )
	{
		m1 : u16 ;
		m2 : u16 ;
		vs16 : s16 ;

		m1 := 1 ;

		aaa := 0 ;

// -1

#line 20 "/home/claudio/HLA/tst/test.hla"
		vs16 := -((((1234+45u16*67u16)/34-!5))%%3-aaa);

		asm {
		    .fstack_word_get vs16
			jsr std.print_s16_dec
			lda #petscii.nl
			jsr kernel.chrout
		};

// -1499

#line 29 "/home/claudio/HLA/tst/test.hla"
		vs16 := -(((89u16*32u16+456/3u16-12%%5))/2);

		asm {
		    .fstack_word_get vs16
			jsr std.print_s16_dec
			lda #petscii.nl
			jsr kernel.chrout
		};

// -213

#line 38 "/home/claudio/HLA/tst/test.hla"
		vs16 := ((((1024/8u16%%3+99*3u16)-512)));

		asm {
		    .fstack_word_get vs16
			jsr std.print_s16_dec
			lda #petscii.nl
			jsr kernel.chrout
		};

// 1323

#line 47 "/home/claudio/HLA/tst/test.hla"
		vs16 := ((((1024/8u16%%3u16+99u16*3u16)+1024)));

		asm {
		    .fstack_word_get vs16
			jsr std.print_s16_dec
			lda #petscii.nl
			jsr kernel.chrout
		};

// 125

#line 56 "/home/claudio/HLA/tst/test.hla"
		aaa := 123+2 ;

		asm {
			jsr std.print_s16_dec
			lda #petscii.nl
			jsr kernel.chrout
		};

	} ;


