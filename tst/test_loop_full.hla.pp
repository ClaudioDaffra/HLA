#line 1 "/home/claudio/HLA/tst/test_loop_full.hla"
fn main ( )
{
    i : u8;
    i := 0;

// Test infinite loop with break


#line 7 "/home/claudio/HLA/tst/test_loop_full.hla"
    loop {
        i++;
        if (i > 5) {
            break;
        };
    };

// Test loop with continue


#line 15 "/home/claudio/HLA/tst/test_loop_full.hla"
    i := 0;
    loop (i < 10) {
        i++;
        if (i < 5) {
            continue;
        };
// This part runs only if i >= 5


#line 22 "/home/claudio/HLA/tst/test_loop_full.hla"
        asm {
            lda #'.'
            jsr $ffd2
        };
    };

    ret 0;
};
