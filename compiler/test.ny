// example syntax, // comments to denote explanation of syntax 
// /* */ comments to denote what would be in actual source file

// we just use the C preprocessor
#include "basic.inc"	// includes DUP, READ, DROP, ADD and such
// these are global functions

decl afunc PUTCHAR;
// asm functions in all CAPS, byte-code functions in lowercase
// just convention, not required

def bfunc puts {
/*Takes:
	char *
Returns:
	void
*/
	DUP,	// DUPlicates what is on the stack
	READ,	// READs out a pointer
	?(	// begin of loop
		PUTCHAR, // consumes one char
		INC,	// INCreases the pointer
		DUP,
		READ,
	?) ,	// end of loop
	DROP,	// DROPs off 0 char
	DROP	// DROPs off remaining pointer
} ;

// ?( and ?) do execute the block is TOS is true
// ?!( and ?!) do execute the block if TOS is false

// ?( and ?) are instructions, ( and ) aren't

def bfunc main {
/*Takes:
	void
Returns:
	int
*/
	"Hello, world!\n",	// stores pointer to string, string is in global data.
	puts,			// calls puts
	0			// pushes 0, the return value of the program
} ;
