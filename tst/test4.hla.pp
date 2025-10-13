#line 1 "/home/claudio/HLA/tst/test4.hla"

ns standard {

	fn print (  ) -> u8
	{
		ret 0 ;
	} ;

} ;


varu16 : u16 ;

/*
	53244
	53243
	53241
	
	53244 +1 := 53245
	53243 +2 := 53245
	53241 +5 := 53246
*/



#line 24 "/home/claudio/HLA/tst/test4.hla"
fn main ( )
{
	vu8   : u8 ;
	vu16  : u16 ;
	vf40  : f40 ;

	pu8   : ^u8 ;
	pu16  : ^u16 ;
	pf40  : ^f40 ;
	addr  : u16 ;

	pString   : ^u8 ;


	pString := "claudio daffra" ;

// ---------------------------------------- pu8


#line 41 "/home/claudio/HLA/tst/test4.hla"
	asm {
		.std_print_nl
	} ;

	pu8 := @vu8;

	asm {
		.fstack_word_get pu8
		jsr std.print_u16_dec
	} ;
// ---------------------------------------- pu16


#line 52 "/home/claudio/HLA/tst/test4.hla"
	asm {
		.std_print_nl
	} ;

	pu16 := @vu16;

	asm {
		.fstack_word_get pu16
		jsr std.print_u16_dec
	} ;
// ---------------------------------------- pf40


#line 63 "/home/claudio/HLA/tst/test4.hla"
	asm {
		.std_print_nl
	} ;

	pf40 := @vf40;

	asm {
		.fstack_word_get pf40
		jsr std.print_u16_dec
	} ;

// ----------------------------------------



#line 76 "/home/claudio/HLA/tst/test4.hla"
	asm {
		.std_print_nl
	} ;

	pu8 := pu8+1;

	asm {
		.fstack_word_get pu8
		jsr std.print_u16_dec
	} ;

// ----------------------------------------



#line 89 "/home/claudio/HLA/tst/test4.hla"
	asm {
		.std_print_nl
	} ;

	pu16 := pu16+1;

	asm {
		.fstack_word_get pu16
		jsr std.print_u16_dec
	} ;

// ----------------------------------------



#line 102 "/home/claudio/HLA/tst/test4.hla"
	asm {
		.std_print_nl
	} ;

	pf40 := pf40+1;

	asm {
		.fstack_word_get pf40
		jsr std.print_u16_dec
	} ;

// ----------------------------------------



#line 115 "/home/claudio/HLA/tst/test4.hla"
	pString := "claudio daffra" ;
	asm {
		.fstack_word_get pString
		jsr std.print_string
	} ;

//--------------------



#line 123 "/home/claudio/HLA/tst/test4.hla"
	standard::print() ;

//--------------------



#line 127 "/home/claudio/HLA/tst/test4.hla"
	ret 0 ;
} ;


