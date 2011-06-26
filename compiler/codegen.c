#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "compiler.h"

extern FILE * yyin;

/*********** driver main function ************/

int main(int argc, char *argv[])
{
	if (argc != 2) { puts("Usage: parser <in_file>"); return(1); }
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
		return(-1);
	}
	block = begin_b;
	end_b = begin_b + 0x400;

	parser();

	int i = 0;
	while (func[++i].u)
		puts(func[i].name);

	free(begin_b);
	return(0);
}
