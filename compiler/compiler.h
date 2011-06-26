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
} func[NUM_F];
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
