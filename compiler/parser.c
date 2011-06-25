#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"

extern char * yytext;
extern FILE * yyin;

/********* error function **********/
void cerror(char * str)
{
	printf("Error in file \"%s\" at line %d: %s.\n", filename, line, str);
	exit(8);
}

/********* data tables *************/

#define NUM_F 0x100
#define NUM_V 0x100
#define NAME_LEN 20

struct
{
	unsigned u : 1; /* used */
	unsigned f : 1; /* 1 if bfunc, 0 if afunc */
	unsigned d : 1; /* 1 if defined (does b point to something sane?) */

	unsigned int * b; /* block, high level syntax tree */
	unsigned int * c; /* "compiled" code, ready to be emitted */
	char name[NAME_LEN];
} func[NUM_V];
int fnum;

struct
{
	unsigned u : 1; /* used */
	unsigned s : 2; /* 0 for byte, 1 for word, 2 for dword */
	unsigned d : 1; /* 1 if defined */
	unsigned i : 1; /* 1 if initialised */
	unsigned p : 1; /* 1 if it is a pointer */

	union
	{
		void * p;
		unsigned int d;
	} data;
	int num; /* size of array */
	char name[NAME_LEN];
} var[NUM_V];
int vnum;

/* block storage */
unsigned int * block;

/* block buildup */
struct
{
	unsigned int st; /* status */
	unsigned int si; /* size of struct in dwords */
	struct
	{
		unsigned int st;
		union
		{
			void * p;
			int i;
			float f;
		} d;
	} d[0x100];
} b[8];

int bnum;

/******* parser *********/

int find_var(char *str)
{
	int i;
	for (i=1; i<NUM_V; i++)
		if (! strncmp(str, var[i].name, NAME_LEN))
			return(i);
	return(0);
}

int find_func(char *str)
{
	int i;
	for (i=1; i<NUM_F; i++)
		if (! strncmp(str, func[i].name, NAME_LEN))
			return(i);
	return(0);
}

int parser(void)
{
	fnum = vnum = 1;
	bnum = 0;

	int curf;
	int curv;

	for (;;) switch (ctok=yylex())
	{
		case 0 : return(0);

		case TOK_DECL : switch(ctok=yylex())
		{
			case TOK_SIZE :
				if ((ctok = yylex()) == '[')
				{
					if (curv = find_var(yytext)) /* variable already declared? */
					{
						if (var[curv].u != 1) cerror("internal compiler error");
						if (var[curv].s != csize) cerror("inconsistent type for variable");
						if (var[curv].p != 1) cerror("array already declared as scalar");
						if ((ctok = yylex()) == TOK_INT)
						{
							if (var[curv].num == 0)
								var[curv].num = cint;
							else if (var[curv].num != cint)
								cerror("inconsistent array size");
							ctok = yylex();
						}
						if (ctok != ']')
							cerror("invalid array size");
					}
					else
					{
						var[vnum].u = 1;
						var[vnum].s = csize;
						var[vnum].d = 0;
						var[vnum].i = 0;
						var[vnum].p = 1;
						strncpy(var[vnum].name, yytext, NAME_LEN);
						if ((ctok = yylex()) == TOK_INT)
						{
							var[vnum].num = cint;
							ctok = yylex();
						}
						if (ctok != ']')
							cerror("invalid array size");
					}
				}
				else if (ctok == TOK_NAME)
				{
					if (curv = find_var(yytext)) /* variable already declared? */
					{
						if (var[curv].u != 1) cerror("internal compiler error");
						if (var[curv].s != csize) cerror("inconsistent type for variable");
						if (var[curv].p != 0) cerror("scalar already declared as array");
					}
					else
					{
						var[vnum].u = 1;
						var[vnum].s = csize;
						var[vnum].d = 0;
						var[vnum].i = 0;
						var[vnum].p = 0;
						strncpy(var[vnum].name, yytext, NAME_LEN);
					}
				}
				else cerror("variable declaration followed by invalid name");
				if (yylex() != ';') cerror("missing semicolon");
				vnum++;
				break;
		} break;

		case '[' : printf("[\n"); break;
		case ']' : printf("]\n"); break;
		case '{' : printf("{\n"); break;
		case '}' : printf("}\n"); break;
		case ';' : printf(";\n"); break;
		case ',' : printf(",\n"); break;

		case TOK_P_O  : printf("(\n"); break;
		case TOK_P_C  : printf(")\n"); break;
		case TOK_PQ_O : printf("?(\n"); break;
		case TOK_PQ_C : printf("?)\n"); break;
		case TOK_PN_O : printf("?!(\n"); break;
		case TOK_PN_C : printf("?!)\n"); break;

		/* TOK_DECL */
		case TOK_DEF  : printf("def\n"); break;
		case TOK_AFUN : printf("afunc\n"); break;
		case TOK_BFUN : printf("bfunc\n"); break;

		case TOK_NAME : printf("Name: %s\n", yytext); break;
		case TOK_STR  : printf("String: %s\n", yytext); break;
		case TOK_INT  : printf("Integer: %d\n", cint); break;
		case TOK_SIZE : printf("Size: "); switch (csize)
		{
			case byte : printf("byte\n"); break;
			case word : printf("word\n"); break;
			case dword : printf("dword\n"); break;
			case qword : printf("qword\n"); break;
		} break;
	}
}

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

	return(parser());
}