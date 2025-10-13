#line 1 "/home/claudio/HLA/tst/test7.hla"



CG0 : 0.0+0.0 ;

sys prova (real:fac2,real:r10,byte:hla_reg9,byte:x,word:ay )
{
// asm { ... }  ;


#line 9 "/home/claudio/HLA/tst/test7.hla"
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

/*	
	//prova(1.2,3.4,-1,2s16,1u16);
	
	//	---------------------------------- && us8		1 
	
	au8 := 34 ;
	bs8 := -45 ;
	
	f := au8 && bs8 ;
	
	asm {
		.std_print_nl
		.fstack_byte_get f
		jsr std.print_u8_dec
	} ;

	//	---------------------------------- || us8		1 
	
	au8 := 34 ;
	bs8 := -45 ;
	
	f := au8 || bs8 ;
	
	asm {
		.std_print_nl
		.fstack_byte_get f
		jsr std.print_u8_dec
	} ;

	//	---------------------------------- && us8		0 
	
	au8 := 0 ;
	bs8 := 0 ;
	
	f := au8 && bs8 ;
	
	asm {
		.std_print_nl
		.fstack_byte_get f
		jsr std.print_u8_dec
	} ;

	//	---------------------------------- || us8 		0
	
	au8 := 0 ;
	bs8 := 0 ;
	
	f := au8 || bs8 ;
	
	asm {
		.std_print_nl
		.fstack_byte_get f
		jsr std.print_u8_dec
	} ;
	
	//
	//
	//
	
	//	---------------------------------- && us16		1 
	
	au16 := 3421 ;
	bs16 := -4567 ;
	
	f := au16 && bs16 ;
	
	asm {
		.std_print_nl
		.fstack_byte_get f
		jsr std.print_u8_dec
	} ;

	//	---------------------------------- || us16		1 
	
	au16 := 3421 ;
	bs16 := -4567 ;
	
	f := au16 || bs16 ;
	
	asm {
		.std_print_nl
		.fstack_byte_get f
		jsr std.print_u8_dec
	} ;

	//	---------------------------------- && us16		0 
	
	au16 := 0 ;
	bs16 := 0 ;
	
	f := au16 && bs16 ;
	
	asm {
		.std_print_nl
		.fstack_byte_get f
		jsr std.print_u8_dec
	} ;

	//	---------------------------------- || us16 		0
	
	au16 := 0 ;
	bs16 := 0 ;
	
	f := au16 || bs16 ;
	
	asm {
		.std_print_nl
		.fstack_byte_get f
		jsr std.print_u8_dec
	} ;
*/

//	---------------------------------- && fac1 		1



#line 139 "/home/claudio/HLA/tst/test7.hla"
asm {
	.std_print_nl
	.std_print_nl
	.std_print_nl
} ;

/*
	f := 1.0 && 2.2 ;

	asm {
		.std_print_nl
		.fstack_byte_get f
		jsr std.print_u8_dec
	} ;
*/

//	---------------------------------- && fac1 		0



#line 156 "/home/claudio/HLA/tst/test7.hla"
	fx:=CG0;
	f := -0.0 && fx ;

	asm {
		.std_print_nl
		.fstack_byte_get f
		jsr std.print_u8_dec
	} ;

//	---------------------------------- && fac1 		0



#line 167 "/home/claudio/HLA/tst/test7.hla"
	f := 1.0 && LF40 ;

	asm {
		.std_print_nl
		.fstack_byte_get f
		jsr std.print_u8_dec
	} ;

//	---------------------------------- const char *



#line 177 "/home/claudio/HLA/tst/test7.hla"
	ps := DFF ;

	ret 0 ;
} ;


