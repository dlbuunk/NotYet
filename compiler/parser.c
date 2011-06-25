#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"

extern char * yytext;
extern FILE * yyin;

/********* memory macros ***********/ 
unsigned int * begin_b;
unsigned int * end_b;
unsigned int * block;
unsigned int * b;

#define ALLOC(N) do { \
	b = block; \
	if ((block+=((N)<<2)) > end_b) \
		if (! realloc(begin_b,(unsigned int)((end_b+=0x400)-begin_b))) { \
			puts("memory allocation failiure"); \
			exit(-1); \
		} \
	} while (0);

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

struct /* function */
{
	unsigned u : 1; /* used */
	unsigned f : 1; /* 1 if bfunc, 0 if afunc */
	unsigned d : 1; /* 1 if defined (does b point to something sane?) */

	unsigned int * b; /* block, high level syntax tree */
	unsigned int * c; /* "compiled" code, ready to be emitted */
	char name[NAME_LEN];
} func[NUM_V];
int fnum;

struct /* variable */
{
	unsigned u : 1; /* used */
	unsigned s : 2; /* 0 for byte, 1 for word, 2 for dword */
	unsigned d : 1; /* 1 if defined */
	unsigned i : 1; /* 1 if initialised */
	unsigned p : 1; /* 1 if it is a pointer */

	union
	{
		void * p;
		unsigned int i;
		float f;
	} data;
	int num; /* size of array */
	char name[NAME_LEN];
} var[NUM_V];
int vnum;

struct /* block */
{
	unsigned int st; /* status */
	unsigned int si; /* size of struct in dwords */
	void * comp; /* compiled representation */
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
} tb[8];

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
	int i;

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
					if ((ctok = yylex()) == ']')
						cint = 0;
					else if (ctok == TOK_INT)
					{
						if ((ctok = yylex()) != ']')
							cerror("invalid array size");
					}
					else
						cerror("invalid array size");
					if ((ctok = yylex()) != TOK_NAME)
						cerror("array declaration followed by invalid name");
					if (curv = find_var(yytext)) /* variable already declared? */
					{
						if (var[curv].u != 1) cerror("internal compiler error");
						if (var[curv].s != csize) cerror("inconsistent type for variable");
						if (var[curv].p != 1) cerror("array already declared as scalar");
						if (cint)
						{
							if (var[curv].num == 0)
								var[curv].num = cint;
							else if (var[curv].num != cint)
								cerror("inconsistent array size");
						}
					}
					else
					{
						var[vnum].u = 1;
						var[vnum].s = csize;
						var[vnum].d = 0;
						var[vnum].i = 0;
						var[vnum].p = 1;
						strncpy(var[vnum].name, yytext, NAME_LEN);
						var[vnum].num = cint;
						vnum++;
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
						vnum++;
					}
				}
				else
					cerror("variable declaration followed by invalid name");
				if (yylex() != ';') cerror("missing semicolon");
				break;

			case TOK_AFUN :
				if ((ctok = yylex()) != TOK_NAME)
					cerror("invalid name for an afunc");
				if (curf = find_func(yytext)) /* function already declared? */
				{
					if (func[curf].u != 1) cerror("internal compiler error");
					if (func[curf].f != 0) cerror("afunc already declared as bfunc");
				}
				else
				{
					func[fnum].u = 1;
					func[fnum].f = 0;
					func[fnum].d = 0;
					strncpy(func[fnum].name, yytext, NAME_LEN);
					fnum++;
				}
				if (yylex() != ';')
					cerror("missing semicolon");
				break;

			case TOK_BFUN :
				if ((ctok = yylex()) != TOK_NAME)
					cerror("invalid name for an bfunc");
				if (curf = find_func(yytext)) /* function already declared? */
				{
					if (func[curf].u != 1) cerror("internal compiler error");
					if (func[curf].u != 1) cerror("bfunc already defined as afunc");
				}
				else
				{
					func[fnum].u = 1;
					func[fnum].f = 1;
					func[fnum].d = 0;
					strncpy(func[fnum].name, yytext, NAME_LEN);
					fnum++;
				}
				if (yylex() != ';')
					cerror("missing semicolon");
				break;

			default :
				cerror("invalid type for declaration");
		} break;

		case TOK_DEF : switch (ctok = yylex())
		{
			case TOK_SIZE :
				if ((ctok = yylex()) == TOK_NAME)
				{
					if (curv = find_var(yytext)) /* variable already declared */
					{
						if (var[curv].u != 1) cerror("internal compiler error");
						if (var[curv].s != csize) cerror("invalid type for variable");
						if (var[curv].d == 1) cerror("variable is defined twice");
						if (var[curv].p == 1) cerror("scalar already declared as an array");
						var[curv].d = 1;
						if ((ctok = yylex()) == TOK_INT)
						{
							var[curv].i = 1;
							var[curv].data.i = cint;
							if (yylex() != ';')
								cerror("missing semicolon");
						}
						else if (ctok == ';')
							var[curv].i = 0;
						else
							cerror("missing semicolon");
					}
					else
					{
						var[vnum].u = 1;
						var[vnum].s = csize;
						var[vnum].d = 1;
						var[vnum].p = 0;
						strncpy(var[vnum].name, yytext, NAME_LEN);
						if ((ctok = yylex()) == TOK_INT)
						{
							var[vnum].i = 1;
							var[vnum].data.i = cint;
							if (yylex() != ';')
								cerror("missing semicolon");
						}
						else if (ctok == ';')
							var[vnum].i = 0;
						else
							cerror("missing semicolon");
						vnum++;
					}
				}
				else if (ctok == '[')
				{
					if ((ctok = yylex()) == ']')
						cerror("array defined without size");
					else if (ctok == TOK_INT)
					{
						if (yylex() != ']')
							cerror("invalid array size");
					}
					else
						cerror("invalid array size");
					if (yylex() != TOK_NAME)
						cerror("invalid variable name");
					/* csize now holds the bit-size of the elems,
					*   cint now holds the array length,
					* yytext now points to the array name */
					if (curv = find_var(yytext))
					{
						if (var[curv].u != 1) cerror("internal compiler error");
						if (var[curv].s != csize) cerror("invalid type for variable");
						if (var[curv].d == 1) cerror("variable is defined twice");
						if (var[curv].p == 0) cerror("array is already declared as a scalar");
						var[curv].d = 1;
						var[cint].num = cint;
						if ((ctok = yylex()) == '{')
						{
							var[curv].i = 1;
							ALLOC(cint + 3);
							var[curv].data.p = b;
							*b = 7;
							*(b+1) = cint +3;
							*(b+2) = ((unsigned int) b) + 3;
							for (i=3;i<(cint+3);i++)
							{
								if ((ctok = yylex()) == TOK_INT)
									*(b+i) = cint;
								else if (ctok == '}')
									break;
								else
									cerror("invalid array initialisation");
								if (yylex() != ',')
									cerror("invalid array initialisation");
							}
							if (yylex() != ';')
								cerror("missing semicolon");
						}
						else if (ctok == ';')
							var[curv].i = 0;
						else
							cerror("missing semicolon");
					}
					else
					{
						var[vnum].u = 1;
						var[vnum].s = csize;
						var[vnum].d = 1;
						var[vnum].p = 1;
						var[vnum].num = cint;
						strncpy(var[vnum].name, yytext, NAME_LEN);
						if ((ctok = yylex()) == '{')
						{
							var[vnum].i = 1;
							ALLOC(cint + 3);
							var[vnum].data.p = b;
							*b = 7;
							*(b+1) = cint +3;
							*(b+2) = ((unsigned int) b) + 3;
							for (i=3;i<(cint+3);i++)
							{
								if ((ctok = yylex()) == TOK_INT)
									*(b+i) = cint;
								else if (ctok == '}')
									break;
								else
									cerror("invalid array initialisation");
								if (yylex() != ',')
									cerror("invalid array initialisation");
							}
							if (yylex() != ';')
								cerror("missing semicolon");
						}
						else if (ctok == ';')
							var[vnum].i = 0;
						else
							cerror("missing semicolon");
						vnum++;
					}
				}
				else
					cerror("variable definition followed by invalid name");
				break;

			case TOK_BFUN :
				break;

			default :
				cerror("invalid type for definition");
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
		/* TOK_DEF */
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

	if (! (begin_b = malloc(0x1000)))
	{
		puts("memory allocation failure");
		return(-1);
	}
	block = begin_b;
	end_b = begin_b + 0x400;

	return(parser());
}
