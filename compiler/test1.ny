def byte[16] test_b;
def word[4] test_w { 0xFEDC, 0xBA98, 0x7654, 0x3210 };
def dword test_d 0x13579ACF;

decl afunc DUP;
decl afunc DROP;

def bfunc test { DUP, ( 42, 2, DROP ) , DROP } ;
