#define	BLKSIZE	4096

#define ARGC_ERROR 1
#define	FNSIZE	128
#define	LBSIZE	4096
#define	ESIZE	256
#define	GBSIZE	256
#define	NBRA	5
#define	KSIZE	9

#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12
#define	CCIRC	15

#define	STAR	01

#define	READ	0
#define	WRITE	1

void readfile(const char* c);
char *getblock(unsigned int atl, int iof);
char *getline_(unsigned int tl);
int advance(char *lp, char *ep);
int append(int (*f)(void), unsigned int *a);
int backref(int i, char *lp);
int cclass(char *set, int c, int af);
void commands(void);
void compile(int eof);
void error(char *s);
int execute(unsigned int *addr);
void exfile(void);
void filename(const char* c);
int getchr(void);
int getfile(void);
int getnum(void);
void global(int k);
void init(void);
unsigned int *address(void);
void newline(void);
void nonzero(void);
void onhup(int n);
void onintr(int n);
void print(void);
void putchr(int ac);
void putd_(void);
void putfile(void);
int putline(void);
void puts_(char *sp);
void quit(int n);
void setwide(void);
void setnoaddr(void);
void squeeze(int i);
int getch_(void);
void search_(const char* re);
void ungetch_(int c);
void drawline(void);
void printcommand(void);
void search_file(const char* filename, const char* searchfor);
void process_dir(const char* dir, const char* searchfor, void (*fp)(const char*, const char*));

/* these two are not in ansi, but we need them */
#define	SIGHUP	1	/* hangup */
#define	SIGQUIT	3	/* quit (ASCII FS) */
