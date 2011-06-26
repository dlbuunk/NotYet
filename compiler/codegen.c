#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "compiler.h"

extern FILE * yyin;

/*********** code generator ******************/
/* generates intermediate code which than
* can be send to emit(), which will output asm
*/
void codegen(void)
{
}

/*********** emitter, outputs asm ************/
void emit(char *fn)
{
	FILE *out;
	if (! (out = fopen(fn, "w")))
	{
		printf("Cannot open file \"%s\"\n", fn);
		fclose(yyin);
		free(begin_b);
		exit(-2);
	}

	

	fclose(out);
}

/*********** driver main function ************/

int main(int argc, char *argv[])
{
	if (argc != 3) { puts("Usage: parser <in_file> <out_file>"); return(1); }
	if (! (yyin = fopen(argv[1], "r")))
	{
		printf("Cannot open file \"%s\"\n", argv[1]);
		return(-2);
	}
	
	strcpy(filename, argv[1]);
	line = 1;

	if (! (begin_b = malloc(0x1000)))
	{
		puts("memory allocation failure");
		fclose(yyin);
		return(-1);
	}
	block = begin_b;
	end_b = begin_b + 0x400;

	parser();

	codegen();

	emit(argv[2]);

	fclose(yyin);
	free(begin_b);
	return(0);
}
