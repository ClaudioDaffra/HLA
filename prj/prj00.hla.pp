#line 1 "/home/claudio/hla_release/prj/prj00.hla"



#line 2 "/home/claudio/hla_release/prj/prj00.hla"

#line 1 "../nHLALib/stdio.hla"

//	stdio 



#line 4 "../nHLALib/stdio.hla"
ns stdio {

	sys putChar(byte:a)
	{
		asm {
			jsr kernel.chrout
		} ;
	} ;

} ;


#line 3 "/home/claudio/hla_release/prj/prj00.hla"

fn main ( )
{
	stdio::putChar('H');
	stdio::putChar('L');
	stdio::putChar('A');
	ret 0;
} ;


