typedef enum
{
	byte,
	word,
	dword,
	qword
} size;

#define TOK_DECL 300
#define TOK_DEF  301
#define TOK_AFUN 302
#define TOK_BFUN 303
#define TOK_P_O  304
#define TOK_P_C  305
#define TOK_PQ_O 306
#define TOK_PQ_C 307
#define TOK_PN_O 308
#define TOK_PN_C 309
#define TOK_SIZE 310
#define TOK_NAME 311
#define TOK_STR  312
#define TOK_INT  313

int ctok;
int cint;
size csize;

char filename[80];
int line;
