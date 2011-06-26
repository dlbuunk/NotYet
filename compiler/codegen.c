#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "compiler.h"

extern FILE * yyin;

extern void cerror(char *);

/*********** code generator ******************/
/* generates intermediate code which than
* can be send to emit(), which will output asm
*/

void comp_block(unsigned int *b)
{
	int i;

	/* some error-checking */
	if (! (*b & 0x01))
		cerror("comp_block() attempted to compile unused block");
	if (*b & 0x04)
		cerror("comp_block() attemptet to compile data array");

	/* compile all sub-blocks */
	for (i=3; i<*(b+1);)
	{
		if (*(b+i++) == 8)
			comp_block((unsigned int *) *(b+i++));
		else i++;
	}
}

void codegen(void)
{
	int i;
	int n = 0;

	/* loop through all functions */
	while (func[++n].u)
	{
		if (! func[n].d)
			continue;
		if (! func[n].f)
			cerror("codegen() attempted to compile afunc");
		comp_block(func[n].b);
	}
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

	/* emit ".data" */
	fprintf(out, "\t.data\n");

	int i;
	int n = 0;
	/* first, loop through global vars */
	while (var[++n].u)
	{
		if (var[n].d)
		{
			fprintf(out, "\t.globl _%s\n", var[n].name);
			if (var[n].i)
			{
				fprintf(out, "_%s:\n", var[n].name);
				if (var[n].p)
				{
					if (var[n].s == byte)
						for (i=0; i<var[n].num; i++)
							fprintf(out, "\t.byte\t0x%X\n", (unsigned char) *(((unsigned int *) var[n].data.p) + 3 + i));
					else if (var[n].s == word)
						for (i=0; i<var[n].num; i++)
							fprintf(out, "\t.word\t0x%X\n", (unsigned short int) *(((unsigned int *) var[n].data.p) + 3 + i));
					else if (var[n].s == dword)
						for (i=0; i<var[n].num; i++)
							fprintf(out, "\t.long\t0x%X\n", *(((unsigned int *) var[n].data.p) + 3 + i));
					else
						cerror("emit() invalid type/size");
				}
				else /* ! var[n].p */
				{
					if (var[n].s == byte)
						fprintf(out, "\t.byte\t0x%X\n", (unsigned char) var[n].data.i);
					else if (var[n].s == word)
						fprintf(out, "\t.word\t0x%X\n", (unsigned short int) var[n].data.i);
					else if (var[n].s == dword)
						fprintf(out, "\t.long\t0x%X\n", var[n].data.i);
					else
						cerror("emit() invalid type/size");
				}
			}
			else /* ! var[n].i */
			{
				if (var[n].s == byte)
					fprintf(out, "\t.lcomm _%s, %d\n", var[n].name, var[n].num);
				else if (var[n].s == word)
					fprintf(out, "\t.lcomm _%s, %d\n", var[n].name, var[n].num << 1);
				else if (var[n].s == dword)
					fprintf(out, "\t.lcomm _%s, %d\n", var[n].name, var[n].num << 2);
				else
					cerror("codegen() invalid type/size");
			}
		}
		else /* ! var[n].d */
			fprintf(out, "\t.extern _%s\n", var[n].name);
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
