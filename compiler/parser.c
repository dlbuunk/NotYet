#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "compiler.h"

extern char * yytext;

/********* error function **********/
void cerror(char * str)
{
	printf("Error in file \"%s\" at line %d: %s.\n", filename, line, str);
	exit(8);
}

/******* parser *********/

int find_var(char *str)
{
	int i;
	for (i=1; i<NUM_V; i++)
		if (! strncmp(str, var[i].name, NAME_LEN))
			if (var[i].u == 1)
				return(i);
	return(0);
}

int find_func(char *str)
{
	int i;
	for (i=1; i<NUM_F; i++)
		if (! strncmp(str, func[i].name, NAME_LEN))
			if (func[i].u == 1)
				return(i);
	return(0);
}

void * do_block(int n)
{
	int i = 0;
	int j;
	int curv;
	int curf;
	switch (ctok)
	{
		case TOK_P_O :
			tb[n].st = 1;
			break;
		case TOK_PQ_O :
			tb[n].st = 0x11;
			break;
		case TOK_PN_O :
			tb[n].st = 0x31;
			break;
	}
	tb[n].si = 3;
	for (;;)
	{
		switch (ctok = yylex())
		{
			case TOK_INT :
				tb[n].d[i].st = 1;
				tb[n].d[i].d.i = cint;
				tb[n].si += 2;
				break;
	
			case TOK_NAME :
				if (curv = find_var(yytext))
				{
					if (var[curv].p == 0)
						tb[n].d[i].st = 4;
					else
						tb[n].d[i].st = 6;
					tb[n].d[i].d.i = curv;
					tb[n].si += 2;
				}
				else if (curf = find_func(yytext))
				{
					tb[n].d[i].st = 2;
					tb[n].d[i].d.i = curf;
					tb[n].si += 2;
				}
				else
					cerror("cannot find variable or function");
				break;

			case TOK_STR :
				ALLOC((strlen(yytext)>>2)+1)
				strcpy((char *)b, yytext);
				tb[0].d[i].st = 7;
				tb[0].d[i].d.p = b;
				tb[0].si += 2;
				break;

			case '&' :
				if (yylex() == TOK_NAME)
				{
					if (curv = find_var(yytext))
					{
						if (var[curv].p == 0)
						{
							tb[n].d[i].st = 5;
							tb[n].d[i].d.i = curv;
							tb[n].si += 2;
						}
						else
							cerror("cannot take adress of array");
					}
					else if (curf = find_func(yytext))
					{
						tb[n].d[i].st = 3;
						tb[n].d[i].d.i = curf;
						tb[n].si += 2;
					}
					else
						cerror("cannot find variable or function");
				}
				else
					cerror("& must be followed by a valid identifier");
				break;

			case TOK_P_O :
			case TOK_PQ_O :
			case TOK_PN_O :
				tb[n].d[i].st = 8;
				tb[n].d[i].d.p = do_block(n+1);
				tb[n].si += 2;
				break;

			default :
				cerror("invalid statement in function body");
		}

		switch (yylex())
		{
			case ',' :
				i++;
				break;

			case TOK_PN_C :
				tb[n].st |= 0x80;
			case TOK_PQ_C :
				tb[n].st |= 0x40;
			case TOK_P_C :
				ALLOC(tb[n].si);
				*b = tb[n].st;
				*(b+1) = tb[n].si;
				for (i=0,j=3;j<tb[0].si;i++)
				{
					*(b+j++) = tb[n].d[i].st;
					*(b+j++) = tb[n].d[i].d.i;
				}
				return(b);

			default :
				cerror("missing comma");
		}
	}
}

void do_func(int f)
{
	int i = 0;
	int j;
	int curf;
	int curv;
	tb[0].st = 3;
	tb[0].si = 3;
	for (;;)
	{
		switch (ctok = yylex())
		{
			case TOK_INT :
				tb[0].d[i].st = 1;
				tb[0].d[i].d.i = cint;
				tb[0].si += 2;
				break;
	
			case TOK_NAME :
				if (curv = find_var(yytext))
				{
					if (var[curv].p == 0)
						tb[0].d[i].st = 4;
					else
						tb[0].d[i].st = 6;
					tb[0].d[i].d.i = curv;
					tb[0].si += 2;
				}
				else if (curf = find_func(yytext))
				{
					tb[0].d[i].st = 2;
					tb[0].d[i].d.i = curf;
					tb[0].si += 2;
				}
				else
					cerror("cannot find variable or function");
				break;

			case TOK_STR :
				ALLOC((strlen(yytext)>>2)+1)
				strcpy((char *)b, yytext);
				tb[0].d[i].st = 7;
				tb[0].d[i].d.p = b;
				tb[0].si += 2;
				break;

			case '&' :
				if (yylex() == TOK_NAME)
				{
					if (curv = find_var(yytext))
					{
						if (var[curv].p == 0)
						{
							tb[0].d[i].st = 5;
							tb[0].d[i].d.i = curv;
							tb[0].si += 2;
						}
						else
							cerror("cannot take adress of array");
					}
					else if (curf = find_func(yytext))
					{
						tb[0].d[i].st = 3;
						tb[0].d[i].d.i = curf;
						tb[0].si += 2;
					}
					else
						cerror("cannot find variable or function");
				}
				else
					cerror("& must be followed by a valid identifier");
				break;

			case TOK_P_O :
			case TOK_PQ_O :
			case TOK_PN_O :
				tb[0].d[i].st = 8;
				tb[0].d[i].d.p = do_block(1);
				tb[0].si += 2;
				break;

			default :
				cerror("invalid statement in function body");
		}

		if ((ctok = yylex()) == ',') i++;
		else if (ctok == '}')
		{
			if (yylex() == ';')
			{
				ALLOC(tb[0].si);
				*b = tb[0].st;
				*(b+1) = tb[0].si;
				for (i=0,j=3;j<tb[0].si;i++)
				{
					*(b+j++) = tb[0].d[i].st;
					*(b+j++) = tb[0].d[i].d.i;
				}
				func[f].b = b;
				return;
			}
			else
				cerror("junk at end of function");
		}
		else
			cerror("missing comma");
	}
}

void parser(void)
{
	int i;

	fnum = vnum = 1;

	int curf;
	int curv;

	for (;;) switch (ctok=yylex())
	{
		case 0 : return;

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
					cerror("invalid name for a bfunc");
				if (curf = find_func(yytext)) /* function already declared? */
				{
					if (func[curf].u != 1) cerror("internal compiler error");
					if (func[curf].f != 1) cerror("bfunc already defined as afunc");
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
								else
									cerror("invalid array initialisation");
								if ((ctok = yylex()) == '}')
									break;
								else if (ctok != ',')
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
								else
									cerror("invalid array initialisation");
								if ((ctok = yylex()) == '}')
									break;
								else if (ctok != ',')
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
				if ((ctok = yylex()) != TOK_NAME)
					cerror("invalid name for a bfunc");
				if (curf = find_func(yytext)) /* function already declared? */
				{
					if (func[curf].u != 1) cerror("internal compiler error");
					if (func[curf].f != 1) cerror("bfunc already defined as afunc");
					if (func[curf].d == 1) cerror("bfunc defined twice");
					func[curf].d = 1;
					if (yylex() != '{')
						cerror("bad function definition");
					do_func(curf);
				}
				else
				{
					func[fnum].u = 1;
					func[fnum].f = 1;
					func[fnum].d = 1;
					strncpy(func[fnum].name, yytext, NAME_LEN);
					if (yylex() != '{')
						cerror("bad function definition");
					do_func(fnum);
					fnum++;
				}
				break;

			default :
				cerror("invalid type for definition");
		} break;

		default :
			cerror("invalid statement at global level");
	}
}
