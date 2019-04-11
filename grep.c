#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "grep.h"
#include <glob.h>

char  Q[]  = "";
int  peekc;
int  lastc;
char  savedfile[FNSIZE];
char  file[FNSIZE];
char  linebuf[LBSIZE];
char  expbuf[ESIZE+4];
int  given;
unsigned int  *addr1, *addr2;
unsigned int  *dot, *dol, *zero;
char  genbuf[LBSIZE];
long  count;
char  *nextip;
char  *linebp;
int  ninbuf;
int  io;
int  pflag;
int  vflag  = 1;
int  oflag;
int  listf;
int  listn;
char  *globp;
int  tfile  = -1;
int  tline;
char  *tfname;
char  *loc1;
char  *loc2;
char  obuff[BLKSIZE];
int  nleft;
int  names[26];
int  nbra;
int  fchange;
unsigned nlall = 128;

char  *mktemp(char *);
char  tmpXXXXX[50] = "/tmp/eXXXXX";

void search_file(const char* filename, const char* searchfor) {
  printf("\n");
  drawline();
  drawline();
  printf("processing %s...\n", filename);
  drawline();
  readfile(filename);
  search_(searchfor);
}

void process_dir(const char* dir, const char* searchfor, void (*fp)(const char*, const char*)) {
  if (strchr(dir, '*') == NULL) {  search_file(dir, searchfor);
    return; }
  glob_t results;
  memset(&results, 0, sizeof(results));
  glob(dir, 0, NULL, &results);
  drawline();
  drawline();
  drawline();
  printf("processing files in %s...\n\n", dir);
  for (int i = 0; i < results.gl_pathc; ++i) {const char* filename = results.gl_pathv[i];
    fp(filename, searchfor);
  }
  globfree(&results);
}

int main(int argc, char *argv[]) {
  if (argc < 3) { fprintf(stderr, "Usage: ./grep searchre file(s)\n");
   exit(ARGC_ERROR); }
  zero = (unsigned *)malloc(nlall * sizeof(unsigned));
  tfname = mktemp(tmpXXXXX);
  init();
  const char* search_for = argv[1];
  process_dir(argv[2], search_for, search_file);
  printf("\n");
  drawline();
  printf("quitting...\n");
  exit(1);
}

#define BUFSIZE 100
char buf[BUFSIZE];
int bufp = 0;

int getch_(void) {  char c = (bufp > 0) ? buf[--bufp] : getchar();
  lastc = c & 0177;
  return lastc;
}

void ungetch_(int c) {  if (bufp >= BUFSIZE) { printf("ungetch: overflow\n");
} else { buf[bufp++] = c;}
}

void search_(const char* re) {char buf[GBSIZE];
  snprintf(buf, sizeof(buf), "/%s\n", re);
  printf("g%s", buf);
  const char* p = buf + strlen(buf) - 1;
  while (p >= buf) { ungetch_(*p--); }
  global(1);
}

void drawline(){	printf("------------------------------------------------------------------\n");}

void readfile(const char* s){	filename(s);
  init();
	if ((io = open(file, 0)) < 0) { lastc = '\n';
    error(file);}
	setwide();
  squeeze(0);
  append(getfile, addr2);
  exfile();
}

void printcommand(void) {  int c;  char lastsep;
  for (;;) {  unsigned int* a1;
    if (pflag) { pflag = 0;
      addr1 = addr2 = dot;
      print(); }
    c = '\n';
    for (addr1 = 0;;) {  lastsep = c;
      a1 = address();
      c = getchr();
      if (c != ',' && c != ';') { break; }
      if (lastsep==',') { error(Q); }
      if (a1==0) {  a1 = zero+1;
        if (a1 > dol) { a1--; }  }
      addr1 = a1;
      if (c == ';') { dot = a1; }
    }
    if (lastsep != '\n' && a1 == 0) { a1 = dol; }
    if ((addr2 = a1)==0) { given = 0;
      addr2 = dot;
    } else { given = 1; }
    if (addr1==0) { addr1 = addr2; }
    switch(c) {
      case 'p':  case 'P':  newline();  print();  continue;
      case EOF:  default:  return;
    }
  }
}

void print(void) {	unsigned int *a1;
	nonzero();
  a1 = addr1;
	do {
		if (listn) {	count = a1-zero;
      putchr('\t');}
		puts_(getline_(*a1++));
	} while (a1 <= addr2);
	dot = addr2;
  listf = 0;
  listn = 0;
  pflag = 0;
}

unsigned int *
address(void) {
	int sign = 1, opcnt = 0, nextopand = -1, c;
  unsigned int *a = dot, *b;
	do {
		do c = getchr();
    while (c==' ' || c=='\t');
		if ('0'<=c && c<='9') {
			peekc = c;
			if (!opcnt)
				a = zero;
			a += sign*getnum();
		} else switch (c) {
		case '$':		a = dol;
		case '.':	if (opcnt){	error(Q);}	break;
		case '\'':	c = getchr();	if (opcnt || c<'a' || 'z'<c){	error(Q);}	a = zero;	do a++; while (a<=dol && names[c-'a']!=(*a&~01));	break;
		case '?':	sign = -sign;
		case '/':		compile(c);	b = a;	for (;;) {	a += sign;	if (a<=zero){	a = dol;} 	if (a>dol){	a = zero;}	if (execute(a)){	break;}  if (a==b){ error(Q);}}	break;
		default:	if (nextopand == opcnt) {		a += sign;	if (a<zero || dol<a){		continue;}}	if (c!='+' && c!='-' && c!='^') {		peekc = c;	if (opcnt==0){	a = 0;}	return (a);
			}
			sign = 1;
			if (c!='+'){	sign = -sign;}
			nextopand = ++opcnt;
			continue;
		}
		sign = 1;
    opcnt++;
	} while (zero<=a && a<=dol);
	error(Q);
	return 0;
}

int getnum(void) {
	int r =0, c;
	while ((c=getchr())>='0' && c<='9'){ r = r*10 + c - '0';}
	peekc = c;
	return (r);
}

void setwide(void) {	if (!given) { addr1 = zero + (dol>zero);
  addr2 = dol;}}

void setnoaddr(void) { if (given) {error(Q);}}

void nonzero(void) {squeeze(1);}

void squeeze(int i) {	if (addr1<zero+i || addr2>dol || addr1>addr2){ error(Q);}}

void newline(void) {
	int c;
	if ((c = getchr()) == '\n' || c == EOF){ return;}
	if (c=='p' || c=='l' || c=='n') {	pflag++;
		if (c=='l'){ listf++;
		}else if (c=='n'){ listn++;}
		if ((c=getchr())=='\n'){ return;}
	}
	error(Q);
}

void filename(const char * c) { strcpy(file, c);
  strcpy(savedfile, c);
}

void exfile(void) {  close(io);  io = -1;
  if (vflag) {  putd_();
  puts_(" characters read");  }
}

void error(char *s) {
	int c;
	listf = 0;
  listn = 0;
  putchr('?');
  puts_(s);
  count = 0;
  lseek(0, (long)0, 2);
  pflag = 0;
	if (globp){	lastc = '\n';}
	globp = 0;
  peekc = lastc;
	if(lastc){	while ((c = getchr()) != '\n' && c != EOF){	;}}
	if (io > 0) {	close(io);
    io = -1;}
}

int getchr(void) {
	char c;
  if ((lastc=peekc)) {	peekc = 0;
		return(lastc);
	}
	if (globp) {
		if ((lastc = *globp++) != 0){	return(lastc);}
		globp = 0;
    return(EOF);
	}
  if ((c = getch_()) <= 0) {	return(lastc = EOF);}
	lastc = c&0177;
  return(lastc);
}

int getfile(void) {
	int c;
  char *lp, *fp;
	lp = linebuf;
  fp = nextip;
	do {
		if (--ninbuf < 0) {
      if ((ninbuf = (int)read(io, genbuf, LBSIZE)-1) < 0) {
				if (lp>linebuf) {  puts_("'\\n' appended");  *genbuf = '\n';  } else { return(EOF); }
      }
      fp = genbuf;
      while(fp < &genbuf[ninbuf]) { if (*fp++ & 0200) { break; } }
      fp = genbuf;
		}
    c = *fp++;
    if (c=='\0') { continue; }
    if (c&0200 || lp >= &linebuf[LBSIZE]) { lastc = '\n';  error(Q);  }
    *lp++ = c;
    count++;
	} while (c != '\n');
	*--lp = 0;
  nextip = fp;
  return(0);
}

void putd_(void) {  int r = count % 10;
  count /= 10;
  if (count) { putd_(); }
  putchr(r + '0');
}

int append(int (*f)(void), unsigned int *a) {
	unsigned int *a1, *a2, *rdot;
  int nline, tl;
	nline = 0;
  dot = a;
	while ((*f)() == 0) {
		if ((dol-zero)+1 >= nlall) {
			unsigned *ozero = zero;
			nlall += 1024;
			if ((zero = (unsigned *)realloc((char *)zero, nlall*sizeof(unsigned)))==NULL) { error("MEM?");}
			dot += zero - ozero;
      dol += zero - ozero;
		}
		tl = putline();
    nline++;
    a1 = ++dol;
    a2 = a1+1;
    rdot = ++dot;
		while (a1 > rdot){	*--a2 = *--a1;}
		*rdot = tl;
	}
	return(nline);
}

void quit(int n) {
	if (vflag && fchange && dol!=zero) { fchange = 0;
    error(Q);	}
	unlink(tfname);
  exit(0);
}

char *
getline_(unsigned int tl) {
	char *bp, *lp;
  int nl;
	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	tl &= ~((BLKSIZE/2)-1);
  while ((*lp++ = *bp++))
	   if (--nl == 0) { bp = getblock(tl+=(BLKSIZE/2), READ);
			  nl = nleft;
		}
	return(linebuf);
}

int putline(void) {
	char *bp, *lp;
	int nl;
	unsigned int tl;
	fchange = 1;
	lp = linebuf;
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl &= ~((BLKSIZE/2)-1);
  while ((*bp = *lp++)) {
		if (*bp++ == '\n') {	*--bp = 0;
      linebp = lp;
      break;}
		if (--nl == 0) {	bp = getblock(tl+=(BLKSIZE/2), WRITE);
      nl = nleft;	}
	}
	nl = tline;
	tline += (((lp-linebuf)+03)>>1)&077776;
	return(nl);
}

char *
getblock(unsigned int atl, int iof) {
	int bno, off;
	bno = (atl/(BLKSIZE/2));
  off = (atl<<1) & (BLKSIZE-1) & ~03;
  return(obuff+off);
}

void init(void) {
	close(tfile);
	tline = 2;
	close(creat(tfname, 0600));
	tfile = open(tfname, 2);
	dot = dol = zero;
}

void global(int k) {
	char *gp;
  int c;
  unsigned int *a1;
  char globuf[GBSIZE];
	if (globp){	error(Q);}
	setwide();
  squeeze(dol>zero);
	if ((c=getchr())=='\n'){	error(Q);}
	compile(c);
  gp = globuf;
	while ((c = getchr()) != '\n') {
		if (c==EOF){	error(Q);}
		*gp++ = c;
		if (gp >= &globuf[GBSIZE-2]){	error(Q);}
	}
	if (gp == globuf){	*gp++ = 'p';}
	*gp++ = '\n';
	*gp++ = 0;
	for (a1=zero; a1<=dol; a1++) {*a1 &= ~01;
		if (a1>=addr1 && a1<=addr2 && execute(a1)==k){	*a1 |= 01;}
	}

	for (a1=zero; a1<=dol; a1++) {
		if (*a1 & 01) {	*a1 &= ~01;
      dot = a1;
      globp = globuf;
      printcommand();
      a1 = zero;}
	}
}

void compile(int eof) {
	int c, cclcnt;
  char *lastep, *ep;
	ep = expbuf;
	if ((c = getchr()) == '\n') { peekc = c;
    c = eof;}
	nbra = 0;
	if (c=='^') { c = getchr();
    *ep++ = CCIRC;}
	peekc = c;
  lastep = 0;
	for (;;) {
		c = getchr();
		if (c == '\n') {	peekc = c;
    c = eof;}
		if (c==eof) {		*ep++ = CEOF;
    return;}
		if (c!='*'){	lastep = ep;}
		switch (c) {
		case '*': *lastep |= STAR;	continue;
		case '$':	*ep++ = CDOL;	continue;
		case '[':	*ep++ = CCL;	*ep++ = 0;	cclcnt = 1;	if ((c=getchr()) == '^') {	c = getchr();	ep[-2] = NCCL;}
			do {	if (c=='-' && ep[-1]!=0) {		if ((c=getchr())==']') {	*ep++ = '-';	cclcnt++;	break;
			}while (ep[-1]<c) {		*ep = ep[-1]+1;
						ep++;
						cclcnt++;
					}
				}
				*ep++ = c;	cclcnt++;
			} while ((c = getchr()) != ']');
			lastep[1] = cclcnt; continue;
		default:	*ep++ = CCHR;	*ep++ = c;
		}
	}
	expbuf[0] = 0;
  nbra = 0;
  error(Q);
}

int execute(unsigned int *addr) {
	char *p1, *p2 = expbuf;
	if (addr == (unsigned *)0) {	if (*p2==CCIRC){ return(0);}
    p1 = loc2;
  }
  else if (addr==zero)	{return(0);
	}else{	p1 = getline_(*addr);}
	do {	if (advance(p1, p2)) { loc1 = p1;
    return(1);}
	} while (*p1++);
	return(0);
}

int advance(char *lp, char *ep) {
	for (;;) switch (*ep++) {
	case CCHR: if (*ep++ == *lp++){ continue;} return(0);
	case CDOL: if (*lp==0){ continue;} return(0);
	case CEOF: loc2 = lp; return(1);
	case CCL:  if (cclass(ep, *lp++, 1)) { ep += *ep;	continue;} return(0);
	case NCCL: if (cclass(ep, *lp++, 0)) {	ep += *ep;	continue;}	return(0);}
}

int cclass(char *set, int c, int af) {
	int n;
	n = *set++;
	while (--n){	if (*set++ == c) { return(af);}}
	return(!af);
}

void puts_(char *sp) {
	while (*sp){	putchr(*sp++);}
	putchr('\n');
}

char	line[70];
char	*linp	= line;

void putchr(int ac) {
	char *lp = linp;
  int c = ac;
  *lp++ = c;
	if(c == '\n' || lp >= &line[64]) { linp = line;
    write(oflag?2:1, line, lp-line);
    return;}
	linp = lp;
}
