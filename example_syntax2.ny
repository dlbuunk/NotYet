/* example program to demonstrate NotYet programming:
*  this program prompts the user for a number and prints its factorial.
*  C-style comments are used to denote comments in actual source.
*  C++-style comments give explanation about the language.
*/

#include "basic.inc"
#include "stdio.inc"

def bfunc fact {
/*Takes:
	int
Returns:
	int
*/
	?!(		// jumps if the param is nonzero
		INC,	// transform 0 on the stack to a 1 as fact(0) == 1
		;	// return
	) ,
	DUP,
	DEC,
	fact,
	MUL
} ;

// or non-recursive:
def bfunc fact_nr {
/*Takes:
	int
Returns:
	int
*/
	?!(
		INC,
		;
	) ,
	DUP,
	DEC,
	?(
		DUP,
		ROT,
		MUL,
		SWAP,
		DEC
	?) ,
	DROP
} ;

// Unsigned integer to string
def bfunc itos {
/*Takes:
	int
	char *
Returns:
	char *
*/
	0,		/* counter to know how many chars we have */
	ROT,
	ROT,		/* the first loop-ROT is not needed the first time */
	(	ROT,
		INC,
		2,
		SELECT,	// select the second elem of the stack and put it top TOS, nondestructive
		10,
		MOD,	// MODulus
		ROT,
		10,
		DIV
	?) ,
	DROP,
	SWAP,
	DUP,
	INC,
	INC,
	SELECT,
	SWAP,
	(	SWAP,
		ROT,
		SWAP,
		DUP,
		ROT,
		0x30,
		ADD,
		SWAP,
		STORE,
		INC,
		SWAP,
		DEC
	?) ,
	DROP,
	DROP
} ;

// String to unsigned integer
def bfunc stoi {
/*Takes:
	char *
Return:
	int
Note: assumes the input chars are actual numbers.
*/
	DUP,
	0,
	SWAP,
	READ,
	?(	SWAP,
		10,
		MUL,
		SWAP,
		0x30,
		SUB,
		ADD,
		SWAP,
		INC,
		DUP,
		ROT,
		SWAP,
		READ
	?) ,
	DROP,
	SWAP,
	DROP
} ;

def byte[8] str;
// str is a word, that functions as a pointer to an array of 8 bytes

def bfunc main {
/*Takes:
	void
Returns:
	int
*/
	"Please input a number: ",
	PUTS,

	str,
	GETS,	// gets does return the pointer
	stoi,
	fact,

	"It's factorial is: ",
	PUTS,
	str,

	SWAP,
	itos,
	PUTS,
	"\n",
	PUTS,

	0
} ;

