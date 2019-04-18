// fvm.cpp
#include <server.h>
#include <fvm.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <wifiboy_lib.h>
/////////////////////////////////////// forth primitives ////////////////////////////////////////////
void FVM::abort(int err, char* msg){ // error reset
	T->err=err, T->errMsg=msg;
	readString();
}
extern void _send();
void FVM::flush(){        // flush out from terminal out buffer to console
	Serial.print(T->tob);
	if(T->html){
		_send();
	}
	T->oEnd = T->tob; //*T->oEnd = 0;
}
void FVM::print(char c){  // print a character to terminal out buffer, flush out if c=='\n' or buffer full
	if(T->oEnd >= T->oLmt) flush(); *T->oEnd++ = c; *T->oEnd = 0; if(c=='\n') flush();
}
void FVM::cr(){ print('\n'); }
void FVM::print(char* p){ // print a string to terminal out buffer until end of string
	while(*p) print(*p++);
}
void FVM::print(char* p, int n){ // print n-char to terminal out buffer
	while(n--) print(*p++);
}
void FVM::qcr(int n) { if(strlen(T->tob)>=tobLmt-n) print("\n"); }
void FVM::printZ	(int v, int n)	{ print(toDecZ(		v,n)); }
void FVM::print		(int v, int n)	{ print(toDec (		v,n)); }
void FVM::print		(int v)			{ print(toDec (		v  )); }
void FVM::printHexZ	(int v, int n)	{ print(toHexZ(		v,n)); }
void FVM::printHex	(int v, int n)	{ print(toHex (		v,n)); }
void FVM::printHex	(int v)			{ print(toHex (		v  )); }
void FVM::printHex	(char*v)		{ print(toHex ((int)v  )); }
void FVM::printHex	(Word*v)		{ print(toHex ((int)v  )); }
void FVM::printHex	(Word**v)		{ print(toHex ((int)v  )); }
/////////////////////////////////////////////////////////////////////////////////////////////////////
// FVM function 01 show the type of given forth word
void FVM::showWordType(Word*w) {
  uint16_t flag=w->flag;
  if(flag&IMMED) print("IMMED ");
  if(flag&COMPO) print("COMPO ");
  if(flag&HIDEN) print("HIDEN ");
  int _code=(int)w->code;
  if(_code==i_doCon) print("constant ");
  else if(_code==i_doVal) print("value ");
  else if(_code==i_doVar) print("variable ");
  else if(_code==i_doCol) print("colon ");
  else print("primitive ");
  print("type word ");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
// FVM function 02 check if given char (ASCII code) is a white space
boolean FVM::isWhiteSpace(char c){ return c==' '||c=='\t'||c=='\n'||c=='\r'; } // check if c is white space
/////////////////////////////////////////////////////////////////////////////////////////////////////
// FVM function 03 check if given token is a floating number
boolean FVM::isFloatingNumber( char *tkn ){ // use /-?(\d+|\d+\.\d*|\.\d+)(e-?\d+)?/ to check tkn
  if( T->base != 10 ) return 0; // base not decimal
  char *p = tkn, c = *p, d = 0;
  if( c == '-' ) c = *(++p);
  while( c>='0' && c <='9' ) d = c, c = *(++p);
  if( c == '.' ){ c = *(++p);
    while( c>='0' && c <='9' ) d = c, c = *(++p);
  }
  if( ! d ) return 0; // no digit in tkn
  if( c ){
  	if( c != 'e' && c != 'E' ) return 0; // illrgal char
  	c = *(++p); d = 0;
  	if( c == '-' ) c = *(++p);
  	while( c>='0' && c <='9' ) d = c, c = *(++p);
  	print("\nlast digit 0x");
  	printHex(d);
  	print(' ');
  	print(d);
  	if( ! d ) return 0; // no digit in Exponential Part
  }
  return 1; }
/////////////////////////////////////////////////////////////////////////////////////////////////////
// FVM function 04 converte given token to a number (integer or floating number)
int	FVM::toNumber(char *token){ int sign = 1, n = 0; // assume the number is positive 0
  int b = T->base;
  char *p = remain = token + ( *token < 0x20 ? 1 : 0 ); // adjust token if it is an nString
  remain = hexPrefix(p);
  if( remain ) b = 16; // hexadecimal
  else remain = p; // no hexPrefix
  char c = *remain; // the first char
//PRINTF("base=%d 1st c='%c' 0x%x\n", b, c, c);
  if( ! c ) return 0; // null string is not a number
  if( c == '-' ){ sign = -1, c = *(++remain); // the number is negative
  //PRINTF("2nd c='%c' 0x%x sign=%d\n",c,c,sign);
  }
  if( b == 10 ){ // decimal
    while( c>='0' && c<='9' ){
    	n = n*10+(c-'0'), c = *(++remain);
    //	PRINTF("next c='%c' 0x%x\n", c, c); // convert digits to an absolute number
    }
    if( ! c ){ n *= sign;
    //	PRINTF("n=%d sign=%d\n", n, sign); // adjust the number if end of token
    }
    else {
      if( isFloatingNumber( p ) ){ // floating number
      	  String tkn = p; X x; x.f = tkn.toFloat(); n = x.i, remain = "";
      //	  PRINTF("n=%e\n", x.f);
      }
      //else PRINTF("n=%d sign=%d remain=\"%s\"\n", n, sign, remain);
    }
  } else {
  	  n = strtol(remain, &remain, b), n *= sign; // convert to integer number on base b (*remain is non-digit)
  	//PRINTF("n=%d 0x%x sign=%d remain=\"%s\"\n", n, n, sign, remain);
  }
  return n; // n is invalid if *remain != 0.
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
// FVM function 05 check if token converted is not a number (integer or floating number)
boolean FVM::isNAN(){ return *remain != 0; };
/////////////////////////////////////////////////////////////////////////////////////////////////////
// FVM function 06 print given integer number and a space
void FVM::dot(int i){ print(toStr(i, T->base)); print(' '); }
/////////////////////////////////////////////////////////////////////////////////////////////////////
// FVM function 07 print integer v as n digits with leading c (given ASCII code of 0 or blank)
void FVM::dotR(int v, int n, char c){ // 
  char*s=toStr(v,T->base); for(int8_t i=strlen(s); i<n; i++) print(c); print(s); }
/////////////////////////////////////////////////////////////////////////////////////////////////////
// FVM function 08 show data stack
void FVM::dotS(){
  int n=dDepth();
if(n<0){ abort(-1, "data stack underflow"); return; }
  print("dDepth="); print(dDepth()); print(" [ "); // show depth
  if(n>0){
    if(n>5)print(".. "); // showing at most top 5 items
    for ( int *i=T->DP-4>T->DS?T->DP-4:T->DS; i <= T->DP; i++ ) dot(*i); // show stack
  }
  print("] base="); print(T->base); print(' ');
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
// FVM function 09 show id and name of given forth word
void	FVM::dotId( Word*w ){ // show word's id and name
  uint8_t n = (uint8_t)*w->name; qcr( n + 11 );
  print('W'); printHexZ((uint16_t)w->id, 3); print(" 0x");
  printHexZ(n, 2); print(' '); print(w->name+1); print(' ');
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
// FVM function 0a show return stack and data stack
void	FVM::showStacks(){ // show rack and stack
  print("\n< "), showTime();
  print("rStk "); print(rDepth()); print(" [ "); // show depth
  if(rDepth()>0){
    if(rDepth()>5)print(".. "); // showing at most top 5 items
    for ( int *i=T->RP-4>T->RS?T->RP-4:T->RS; i<=T->RP; i++ ) dot(*i); // show rack
  }
  print("] ");
  dotS();
  print("inp"); print(lineIndex++); print(" >\n"); // show base 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
boolean FVM::dIsFull   ()      { return T->DP   >= T->DS+DS_SIZE; } // check if stack full
boolean FVM::dHasSpace (int n) { return T->DP+n <= T->DS+DS_SIZE; } // check if stack has space for n items
boolean FVM::dHasItems (int n) { return T->DP   >= T->DS+n-1    ; } // check if stack has n items
void    FVM::dClear    ()      { T->DP=T->DS-1					; } // reset stack
int     FVM::dTop	   ()	   { return *(T->DP)				; }	// get top integer of stack
void    FVM::dTop	   (int n) { *(T->DP)=n						; }	// replace top of stack by given integer
void    FVM::dPush     (int n) { if(! dHasSpace(1)){ abort(-1, "dPush overflow"); return; }
		*(++(T->DP))=n; } // push integer onto stack
int     FVM::dPop      ()      { if(dDepth()<1){ abort(-2, "dPop underflow"); return 0; }
		return *(T->DP--); } // pop integer from stack
int     FVM::dPick     (int i) { if(! dHasItems(i+1)){ abort(-3, "dPick underflow"); return 0; }
		return *(T->DP-i) ; } // top i-th integer of stack
void    FVM::dRoll     (int n) { if(! dHasItems(n+1)){ abort(-3, "dRoll underflow"); return; }
		int *p=T->DP-n, X=*p; while(++p <= T->DP) *(p-1)=*p; *(p-1)=X; } // roll top n integers on stack
void    FVM::dBackRoll (int n) { if(! dHasItems(n+1)){ abort(-3, "dBackRoll underflow"); return; }
		int *p=T->DP, X=*p; while(--p >= T->DP-n) *(p+1)=*p; *(p+1)=X; } // back roll top n integers of stack
int     FVM::dDepth    ()      { return T->DP-T->DS+1; } // depth of stack
/////////////////////////////////////////////////////////////////////////////////////////////////////
boolean FVM::rHasItems (int n) { return T->RP   >= T->RS+n-1    ; } // check if rack has n items
void    FVM::rClear    ()      { T->RP=T->RS-1 ; } // reset rack
void    FVM::rPush     (int n) { *(++(T->RP))=n; } // push integer onto rack
int     FVM::rPop      ()      { return *(T->RP--) ; } // pop integer from rack to data stack
int     FVM::rPick     (int i) { return *(T->RP-i); } // pick i th integer of rack to data stack
int     FVM::rDepth    ()      { return T->RP-T->RS+1; } // depth of rack
/////////////////////////////////////////////////////////////////////////////////////////////////////
char    FVM::toDigit   (int i) { return i+(i<10?0x30:0x57); } // convert integer i to char, for example, 10 to 'a', 35 to 'z'.
/////////////////////////////////////////////////////////////////////////////////////////////////////
char*   FVM::toStr (int i, int b) { // convert int i to to string according to base b
  char*p = tmp+TMP_SIZE-1; *p = 0; // setup buffer pointer p to zStr NULL
  if(i==0) *(--p)='0';
  else {
    boolean negDcimal = i<0 && b==10;
    if( negDcimal ) i = -i;
    uint u = i; // consider i as unsigned int if base is not 10
    while(u) *(--p) = toDigit( u%b ), u /= b; // convert u%b to digit
    if( negDcimal ) *(--p) = '-';
  }
  return p;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char*   FVM::toStr (int i, int b, int n, char c) { // convert int i to base b str at least n-char width (leading c if needed)
  char*p = toStr(i, b); n -= strlen(p);
  while(n-- > 0) *(--p) = c; // at least n bytes
  return p;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char*   FVM::toDec (int i) { // convert integer i into digits of base 10
	return toStr(i, 10);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char*   FVM::toDec(int i, int n) { // convert int i in decimal to zStr at least n-char width (leading ' ' if needed)
  return toStr(i, 10, n, ' ');
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char*   FVM::toDecZ(int i, int n) { // convert int i in decimal to zStr at least n-char width (leading '0' if needed)
  return toStr(i, 10, n, '0');
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char*   FVM::toHex (int i) { // convert unsigned int i in hexadecimal to zStr
  return toStr(i, 16);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char*   FVM::toHex(int i, int n) { // convert unsigned int i in hexadecimal to zStr at least n-char width (leading ' ' if needed)
  return toStr(i, 16, n, ' ');
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char*   FVM::toHexZ(int i, int n) { // convert unsigned int i in hexadecimal to zStr at least n-char width (leading '0' if needed)
  return toStr(i, 16, n, '0');
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char*	  FVM::hexPrefix(char *s) { // leading 0x, 0X, or $ are all acceptable as hexadecimal prefix
  char c;
  if((c=*s++) != '0' && c != '$') return 0;
  if(c=='0' && (c=*s++) != 'x' && c != 'X') return 0;
  return s; // remain string
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char*   FVM::uniqueNStr(char *s) { // save s in the string buffer and return nStr address p
  char *p=SB+1;                 	// let p point to the first string
  if( *s >= 0x20 ) s--;
  uint8_t n=strlen(s+1);
  while (p<sbEnd) {             	// if p is not pointing to end of the string buffer
    if(strcmp(p,s+1)==0) return p-1;// return p if s is already in the string buffer
    p+=*(p-1)+2;                	// let p point to the next string
  }
  if (n > SL_LIMIT) {           	// if string length over limit
    abort( 1, "string length > SL_LIMIT"); return 0;
  }
  if(sbEnd+n >= SB_LIMIT){      	// if the string buffer full
    abort( 2, "string buffer full"); return 0;
  }
  p=sbEnd;                      	// p is pointing to the the string copy
  *sbEnd++=n;                   	// the gap count for going to next string
  strcpy(sbEnd,s+1);             	// append s to the string buffer
  sbEnd+=n+1;                   	// advance the end of the string buffer
  return p;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
Word* FVM::createWord(uint16_t flag, uint16_t id, char*name, FuncP code, int data){
  // create a forth word of name n with the function code f to execute
  // PRINTF("\ncreateWord flag 0x%x id 0x%x name 0x%x \"%s\" code 0x%x data 0x%x",
  //	flag, id, (int)*name, name+1, (int)code, data);
    char*u;
    if( *name >= 0x20 ){
    	*tmp = strlen( name ), strcpy( tmp+1, name );
    	name = tmp;
    }
    Word *w=vocSearch(name);
    if(w){
    	if(T->warning)
    		print("\nwarning !!! 0x"), printHex(*name), print(" \""), print(name+1), print("\" reDef "), u=w->name;
    } else u=uniqueNStr(name);
    T->last=w=(Word*)malloc(sizeof(Word));
    w->flag=flag, w->name=u, w->code=code;
    w->p.con=data;
    if(isColonType(w)!=1) vocAdd(w);
    return w;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FVM::forgetWord(char* name){
	Word *w=vocSearch(name);
	if(w){
	  int f=(int)voc->first, p=(int)voc->predefined;
	  int inFence = (int)w >= p && (int)w <= f;
	  if(inFence){
	    print("\nwarning !!! 0x"), printHex(*name), print(" \""), print(name+1), print("\" in fence, cannot be forgot ");
	    return;
	  }
	  if(isColonType(w)) free(w->p.wpl);
	  T->last=voc->context=w->link, free(w);
	  if(T->warning) print("\nwarning !!! 0x"), printHex(*name), print(" \""), print(name+1), print("\" forgot ");
	} else if(T->warning) print("\nwarning !!! 0x"), printHex(*name), print(" \""), print(name+1), print("\" unDef ");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
Word * FVM::newPrimitive(char*name, FuncP code, char*codeName){
	createWord( 0, 0, uniqueNStr(name), code, (int) codeName ); }
/////////////////////////////////////////////////////////////////////////////////////////////////////
Word * FVM::newPrimitive(char*name, FuncP code){
	char*s = uniqueNStr(name);
	newPrimitive( s, code, s+1 ); }
///////////////////////////////////////////////////////////////////////////////////////
Word * FVM::newConstant(char*name, int i){
	createWord( 0, 0, uniqueNStr(name), (FuncP)i_doCon, i ); }
///////////////////////////////////////////////////////////////////////////////////////
Word * FVM::newVariable(char*name, int*a){
	createWord( 0, 0, uniqueNStr(name), (FuncP)i_doCon, (int)a ); }
///////////////////////////////////////////////////////////////////////////////////////
Task * FVM::createTask( int sizeTob, int sizeTib, int sizeDS, int sizeRS, int sizeCS ){
  bytesForTask = sizeof(Task), bytesForTob = sizeTob, bytesForTib = sizeTib;
  bytesForDS = sizeDS*sizeof(int), bytesForRS = sizeRS*sizeof(int), bytesForCS = sizeCS*4;
  bytesTotal = bytesForTask + bytesForTob + bytesForTib + bytesForDS + bytesForRS + bytesForCS;
  T = (Task *) malloc(bytesTotal);
  int a = (int)T; a += bytesForTask;
  T->tob = (char *) a; a += bytesForTob; T->oEnd = T->tob; T->oLmt = (char *) (a-1);
  T->tib = (char *) a; a += bytesForTib;
  T->DS = (int *) a; a += bytesForDS;
  T->RS = (int *) a; a += bytesForRS;
  T->CS = (Word **) a, T->base = 10, T->err = 0;
  return T;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
Task * FVM::createTask(){ return createTask( TOB_SIZE, TIB_SIZE, DS_SIZE, RS_SIZE, CS_SIZE ); }
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool    FVM::isWord (Word*w) { // check if  w is a word
    Word*x=voc->context; while(x){ if((int)x==(int)w) return 1; x=x->link; } return 0; }
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::cpInit () { T->CP=T->CS; } // initialize temporary compile space
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::compile (Word*w) { // compile w into temporary compile space
if(T->tracing) printHex(T->CP), print(':'), printHex(w), print(' '); *((T->CP)++)=w; }
/////////////////////////////////////////////////////////////////////////////////////////////////////
Word**  FVM::cpClone() { // clone temporary colon word-list as a new colon word-list
    int n=(T->CP)-(T->CS), m=n*sizeof(Word*);
    Word** wplist=(Word**)malloc(m);
    if(T->tracing) print("\ncpClone() malloc("), print(m), print(") at 0x"), printHex(wplist), print('\n');
    memcpy(wplist,T->CS,m);
    return wplist;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FVM::vocInit (Word *wrd) { // link given word set to dictionary
  Word *w=wrd, *first;
  voc = (Voc*)malloc(sizeof(Voc));
  voc->predefined = wrd;
  voc->context = T->last = wrd; // initialize vocabulary (dictionary)
  voc->nWord=0, voc->firstNew=0;
  while(w) first=w, voc->nWord++, w=w->link;
  voc->first = first;
  voc->lastId=T->last ? T->last->id : 0;
  if(T->tracing){
    print('\n'); print(voc->nWord); print(" forth words predefined");
    if(voc->nWord){
  	  print("\nthe first at 0x"), printHex(first);
  	  print(" is 0x"), printHex(*(first->name));
  	  print(" \""), print(first->name+1), print("\"");
  	  print("\nthe fence at 0x"), printHex(wrd);
  	  print(" is 0x"), printHex(*wrd->name);
  	  print(" \""), print(wrd->name+1), print("\"");
    }
  }
  w_doCon=vocSearch("\x05" "(con)"); if(w_doCon) i_doCon=w_doCon->p.con;
  w_doVal=vocSearch("\x05" "(val)"); if(w_doVal) i_doVal=w_doVal->p.con;
  w_doVar=vocSearch("\x05" "(var)"); if(w_doVar) i_doVar=w_doVar->p.con;
  w_doCol=vocSearch("\x03" "(:)"  ); if(w_doCol) i_doCol=w_doCol->p.con;
  w_ret  =vocSearch("\x03" "(;)"  ); if(w_ret  ) i_ret=(int)w_ret->code;
  if(T->tracing){
    print("\nw_doCon=0x"), printHex(w_doCon), print(", w_doVal=0x"), printHex(w_doVal);
    print(", w_doVar=0x"), printHex(w_doVar), print(", w_doCol=0x"), printHex(w_doCol);
    print("\ni_doCon=0x"), printHex(i_doCon), print(", i_doVal=0x"), printHex(i_doVal);
    print(", i_doVar=0x"), printHex(i_doVar), print(", i_doCol=0x"), printHex(i_doCol);
    print("\nw_ret=0x"), printHex(w_ret), print(", i_ret=0x"), printHex(i_ret); print(' ');
  }
  w_doBegin =vocSearch("\x07" "(begin)" );
  w_doAgain =vocSearch("\x07" "(again)" );
  w_doUntil =vocSearch("\x07" "(until)" );
  w_doWhile =vocSearch("\x07" "(while)" );
  w_doRepeat=vocSearch("\x08" "(repeat)");
  w_doThen  =vocSearch("\x06" "(then)"  );
  w_doFor   =vocSearch("\x05" "(for)"   );
  w_doNext  =vocSearch("\x06" "(next)"  ); needExtraCell[0]=i_doNext = w_doNext ? (int)w_doNext ->code : 0;
  w_doIf    =vocSearch("\x04" "(if)"    ); needExtraCell[1]=i_zbran  = w_doIf   ? (int)w_doIf   ->code : 0;
  w_doElse  =vocSearch("\x06" "(else)"  ); needExtraCell[2]=i_bran   = w_doElse ? (int)w_doElse ->code : 0;
  w_doLit   =vocSearch("\x05" "(lit)"   ); needExtraCell[3]=i_doLit  = w_doLit  ? (int)w_doLit  ->code : 0;
  w_compile =vocSearch("\x07" "compile" ); needExtraCell[4]=i_compile= w_compile? (int)w_compile->code : 0;
  if(T->tracing){
	print("\nw_doNext=0x"), printHex(w_doNext), print(", w_doIf=0x"), printHex(w_doIf);
	print(", w_doElse=0x"), printHex(w_doElse), print(", w_doLit=0x"), printHex(w_doLit);
	print(", w_compile=0x"), printHex(w_compile);
	print("\ni_doNext=0x"), printHex(i_doNext), print(", i_zbran=0x"), printHex(i_zbran);
	print(", i_bran=0x"), printHex(i_bran), print(", i_doLit=0x"), printHex(i_doLit);
	print(", i_compile=0x"), printHex(i_compile);
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FVM::vocAdd (Word *w) { // add forth word w into dictionary
  if(! voc->firstNew) voc->firstNew=w;
  w->id = ++(voc->lastId); w->link = voc->context; voc->context=w;
  if(T->tracing) print("\nnew forth word"), see(w), print('\n');
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
Word *FVM::vocSearch (char *token) { // check if given nStr is the name of a forth word
  Word *w=0;
  if(token){ char n=*token;
    if(n){
      if(n>=0x20){
        abort( 3, "token len >= 0x20" ); return 0;
      }
      w = voc->context;
      while ( w && strcmp(w->name,token) ){
        w=w->link;
      }
    }
  }
  return w;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::words(char*sub) { // show all word names having specific substring
  char *s=sub; if(s && *s<0x20) s++;
  char n; int i=voc->nWord;
  Word* w=voc->context;
  while (w) {
    if( s==0 || *s==0 || strstr(w->name+1,s) ){
      if( !(w->flag&HIDEN) ){
        qcr(int(*w->name)+6), print('W'), printHexZ(w->id,3), print(' '), print(w->name+1), print(' ');
      }
    }
    w=w->link, i--; 
  }
  print('\n');
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::dump(int *a,int n) { // dump n cells at adr
    int *lmt=a+n;
    char *ba;
    char *blmt;
    for( ; a<lmt; a+=4) {
      printHexZ((int)a,8), print(": ");
      for(int i=0; i< 4; i++){
        if( a+i>=lmt )print("         ");
        else          printHexZ((int)*(a+i),8), print(' ');
      }
      ba=(char*)a, blmt=(char*)lmt;
      for(int i=0; i<16; i++){
        if(ba+i>=blmt)print("   ");
        else          printHexZ((int)*(ba+i),2), print(' ');
      }
      for(int i=0; i<16; i++){
        if(ba+i>=blmt) print(' ');
        else {
          char c=*(ba+i); n=(int)c;
          if( n==0 ) c='.';
          else if( n<0x20 || (n>0x7e&&n<0xa4) || n>0xc6 ) c='_';
          else if(n>=0xa4&&n<=0xc6) { // head-byte of commmon big5
            n=(int)*(ba+i+1);
            if( n<0x40 || (n>0x7e&&n<0xa1) || n>0xfe) c='_'; // next is not tail-byte of commmon big5 
            else print(c), c=(char)n, i++; // show head-byte of commmon big5 and get tail-byte
          }
          print(c);
        }
      }
      print("\n");
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void FVM::mountFFS(){
  if( ! SPIFFS.begin(-1) ){
  	abort( 4, "SPIFFS Mount Failed"); return;
  }
  if(hint) PRINTF( "\nSPIFFS totalBytes %d usedBytes %d\n", SPIFFS.totalBytes(), SPIFFS.usedBytes() );
  char *path = "/";
  File dir = SPIFFS.open( path, FILE_READ );
  if( ! dir ){ print("\nno SPIFFS directory yet "); return; }
  curDir = dir;
  if(hint) PRINTF( "\nSPIFFS current directory %s ", dir.name() );
//fileSystemTest();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FVM::fileSystemTest(){
  if( ! curDir ){ print("\nno SPIFFS directory "); return; }
  PRINTF( "\nshow directory %s ", curDir.name() );
  int nfiles=0, dirsize=0, totalsize=0, filesize;
  char filename[20],dirname[20];
  File curfile;
  while( curfile = curDir.openNextFile(FILE_READ) ) {
    strcpy(filename,curfile.name());
    char *p=strchr(filename+1,'/');
    if(p){
      if(!nfiles){ *p=0; strcpy(dirname,filename); }
      nfiles++, filesize=curfile.size(); dirsize+=filesize, totalsize+=filesize;
    } else {
      if(nfiles){
        print("\n   <DIR> %s %d files %d (0x%x) bytes ",dirname,nfiles,dirsize,dirsize);
        dirsize=nfiles=0;
      }
    //PRINTF(" 0x%x %s %d %c\n",curfile.size(),filename,p-filename,*p);
    }
  	PRINTF( "\n%8d %s ", curfile.size(), curfile.name() );
  }
  if(nfiles) PRINTF( "\n%8d %s <DIR> %d files ", dirsize, dirname, nfiles );
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////
const bool any(int w, int*ws, int8_t n){ while(n--){ if(w==ws[n]) return 1; } return 0; }
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool FVM::isConstantType(Word*w){ return w?w->code==(FuncP)i_doCon:0; }
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool FVM::isValueType(Word*w){ return w?w->code==(FuncP)i_doVal:0; }
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool FVM::isVariableType(Word*w){ return w?w->code==(FuncP)i_doVar:0; }
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool FVM::isColonType(Word*w){ return w?w->code==(FuncP)i_doCol:0; }
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool FVM::isPrimitiveType(Word*w){
  if(isConstantType(w)) return 0;
  if(isValueType(w)) return 0;
  if(isVariableType(w)) return 0;
  if(isColonType(w)) return 0;
  return 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FVM::see(Word *w) { // show the forth word
  if(! isWord(w)){
  	abort(7, "unDef"); return;
  }
  int _code=(int)w->code; char *name, n; uint16_t flag;
  print("----------------------");
  Word *link = w->link;
  print('\n'), printHex(&w->link), print(':'), printHexZ((int)link,8), print(" link ");
  if(link) print("to W"), printHexZ(link->id,3), print(' ');
  print('\n'), printHex((int)&w->id), print(':'), printHexZ(w->flag,4), printHexZ(w->id,4), print(" flag ");
  showWordType(w);
  print("W"), printHexZ(w->id,3), print(' ');
  name=w->name, n=*name;
  print('\n'), printHex((int)&w->name), print(':'), printHex(name), print(" name 0x"), printHex(*name), print(" \""), print(name+1), print("\" ");
  print('\n'), printHex((int)&w->code), print(':'), printHex(_code), print(" code ");
  if( _code ){
    if(_code==i_doCon) print("_doCon ");
    else if(_code==i_doVal) print("_doVal ");
    else if(_code==i_doVar) print("_doVar ");
    else if(_code==i_doCol) print("_doCol ");
    else print(w->p.mne), print(' ');
  }
  int parm=w->p.con;
  print('\n'), printHex((int)&w->p.con), print(':'), printHexZ(parm,8), print(" parm ");
  if( parm ){
         if(parm==i_doCon) print("_doCon ");
    else if(parm==i_doVal) print("_doVal ");
    else if(parm==i_doVar) print("_doVar ");
    else if(parm==i_doCol) print("_doCol ");
    else if(_code!=i_doCon && _code!=i_doVal && _code!=i_doVar  && _code!=i_doCol) print('"'), print(w->p.mne), print("\" ");
  }
  print("\n----------------------\n");
  if (_code && _code==i_doCol){
    int x_code;
    Word **ip=w->p.wpl, *x;
    do {
       x=*ip; x_code=(int)x->code;
       char*p = x->flag==IMMED? (char*)"[compile] " : (char*)"";
       printHex(ip++), print(':'), printHex(x), print(' '), print(p), print(x->name+1), print(' ');
       if( any( x_code, needExtraCell, 5 ) ){
          Word*z=*ip;
          print('\n'), printHex(ip++), print(':'), printHexZ((int)z,8), print(' ');
          if( x_code==i_compile ) print(z->name+1), print(' ');
          else{
            if( 0x74732805==*(int*)(x->name) ){
              char*p=(char*)z;
              print("0x"), printHex((int)*p), print(" \""), print(p+1), print("\" ");
            } // string of (str)
            else{ print((int)z), print(' ');
              if( any( x_code, needExtraCell, 3 ) ) print("to 0x"), printHex((int)(ip-1+(int)z));
        	}
          }
       }
       cr();
    } while ( x != w_ret );
    print("----------------------\n");
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void 	FVM::callColWrd(){ ipPush(), T->IP = T->W->p.wpl; }
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::ms(int n){ if( T->waitMsUntil == 0 ) T->waitMsUntil = millis(); T->waitMsUntil += n; }
// set to wake up until give time (milli seconds)
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::callPerWrd () { // execute word one by one in the wplist of a colon word
  if( millis() <= T->waitMsUntil ) return; // wake up until given time
  if ( T->IP == 0 ) return; // end of full calling
  T->W=*T->IP++; // from IP, get a word to execute
  int code=(int)T->W->code;
  if(code == 0) return;
  if ( T->tracing ) {
    int8_t nd=dDepth(), nr=rDepth(), n=nr/2, i;
    print("R "), print(nr),print(" [ ");
    if(nr>=2) printHexZ(*(T->RP-1),8),print(' '); else print("........ ");
    if(nr>=1) printHexZ(*(T->RP  ),8),print(' '); else print("........ ");
    print("] D "), print(nd),print(" [ ");
    if(nd>=2) printHexZ(*(T->DP-1),8),print(' '); else print("........ ");
    if(nd>=1) printHexZ(*(T->DP  ),8),print(' '); else print("........ ");
    print("] "),printHex(T->IP-1),print(':'),printHex(T->W),print(' ');
    for(i=0; i<n; i++) print("| ");
    print(T->W->name+1),print(' ');
    if( any( code, needExtraCell, 5 ) ){
    	Word **ip=T->IP; int z=(int)*ip;
    	print(z), print(' ');
    	if( any( code, needExtraCell, 3 ) ) print("0x"), printHex((int)(ip+z));
    }
    cr();
  }
  (T->W)->code(); // execute the forth word W
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::ipPush(){ rPush((int)(T->IP)); rPush((int)(T->W)); }
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::ipPop (){ T->W=(Word*)rPop(); T->IP=(Word**)rPop(); }
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::evalScript (char *str) {
  T->pBgn = T->pEnd = str;
  if ( T->tracing )
    print("\n000 evalScript "), print((int)strlen(str)), print("-byte string at 0x"), printHex(str),
	print(" \""), print(str), print("\" state=0x"), printHex(T->state), print("\n");
  if( T->state & READING ) T->state -= READING; // no longer in reading
  print("\n #tib="), print((int)strlen(str)), print(" > "), flush();
  char *b=str, *p=b, *e;
  while( e = strstr(p,"  ") ){
//	strncpy(T->tib,b,e-b); T->tib[e-b]=0; // parse a line to eval
  	while( b<e ) print(*b++); print('\n'), e++; p=b=e;
  	while( *p && *p==' ' ) p++;
  }
  while( *b ) print(*b++);
  print("\n"); 
  int n=strlen(str); char *pLmt;
  pLmt = str+n, T->state |= PARSING;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::evalPerTkn() { // parse token to evaluate
  if( millis() <= T->waitMsUntil ) return; // wake up until given time
  if( T->state & READING ) return; // in reading chars
  if( T->IP ) return; // in calling words
  if( *(T->pEnd)==0 ) {
	if( T->state & PARSING ) T->state -= PARSING;
    readString(); return; }  // end of parsing
  char *token = parseToken(' ');              // get a token
  if( ! *token ){                  return; }  // end of script ( need to work for fload )
  evalToken( token );                     		  // evaluate token
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::evalToken( char *token ) { // evaluate nString token
  int tracing = T->tracing;
  T->W = vocSearch( token );			// search token in vocabulary
  if ( T->W ) {							// word W found
  	uint16_t flag = T->W->flag, immed = flag & IMMED, compo = flag & COMPO;
	bool notCmpling = ( T->state & CMPLING ) == 0;
    if( immed || notCmpling ) {			// immediate word or state not compilling
      if( compo && notCmpling ) { abort( 6, "compile-only"); return; }
      if( tracing ) print( "execute " ), showWordType(T->W);
	  if( T->W->code == 0 ) return;		// do nothing
      T->W->code();						// execute word W
    }
    else {
      if( tracing ) print( "compile " ), showWordType(T->W);
      compile( T->W );					// compile word W
    }
    return;
  }
  int n = toNumber( token );			// convert token to number
  if( isNAN() ){
  	abort( 7, "unDef" ); return;
  }
  if( T->state & CMPLING ){				// in compilling
    if( T->tracing ) print( "compile " );
    compile( w_doLit ), compile( (Word*)n );   // compile (lit) and number n
  } else {
  	if( tracing ) print( "push " );
  	dPush(n);							// push number to s
  }
  if( tracing ) print("0x"), printHex(n), print(' '), print(n), print(' ');
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char*   FVM::parseToken (char delimiter){ // parse token by given delimiter
//	(if delimiter=0x20, take white space as delimiter)
  char *p, i, n, c=*(T->pEnd);
  if(c==0) return 0;
  if(delimiter==0x20){
    while ( c && isWhiteSpace(c) ) c = *(++(T->pEnd));   // ignore leading white spaces
    T->tokenAt = T->pEnd;                       // token found at first non white space
    while ( c && !isWhiteSpace(c) ) c = *(++(T->pEnd));  // colect non white spaces
    T->tokenEnd=T->pEnd;
  } else {
    if( isWhiteSpace(c) ) T->pEnd++;   // skip leading white space
    T->tokenAt=T->pEnd;
    do { T->tokenEnd=strchr(T->pEnd, delimiter); T->pEnd=T->tokenEnd+1; c=*(T->pEnd);
    } while( c && !isWhiteSpace(c) );
  }
  n = T->tokenEnd - T->tokenAt;               // compute token length
  if ( n>=TMP_SIZE-1 ) {                      // check if length too long
    abort( 8, "token length >= TMP_SIZE-1" );
    return "";
  }
  strncpy(&tmp[1],T->tokenAt,n), tmp[0]=n, tmp[n+1] = 0;         // make a null ended string at tmp
  if( T->tracing ) print('\n'), printHexZ(T->tokenAt-T->pBgn,3), print(" evalPerTkn 0x"), printHex(n), print(" \""), print(&tmp[1]), print("\" ");
  char*u=uniqueNStr(tmp);        // make a null ended string at tmp
  return u;                  // return a unique nString in string buffer
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::showTime(){
  int ms=millis(), s=ms/1000, m=s/60, h=m/60;  ms=ms%1000, s=s%60, m=m%60;
  printZ( h, 2 ), print( ':' ), printZ( m, 2 ), print( ':' ), printZ( s, 2 ), 
  print( '.' ), printZ( ms, 3 ), print( " cpu" ), print( (int)xPortGetCoreID() );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void    FVM::readString() { // read input chars from serial port
  if( ( T->state & READING ) == 0 ) T->state += READING; // now in reading
  T->iEnd = T->tib, T->waitMsUntil = 0, T->IP = 0;
  if( hint ){
  	  print( " < #tob="), print( (int) strlen( T->tob ) - 8 ), cr(), showTime(), print( ' ' );
  	  dotS();     // showing depth, numbers, and number coversion base of data stack 
  }
  if( T->err ){
    print(" !! ERROR "), print(T->err), print(" !!\n "), print(T->errMsg);
	T->err=0, T->state = READING; // no longer parsing or compiling
    char *b=T->tokenAt, *e=T->tokenEnd; int n=e-b; Word *w=T->W, **ip=T->IP;
    print(" token 0x"), printHex(n), print(" \""), print(b, n), print("\" at "), print(b - T->pBgn), print(" of tib");
    if(w){ print(" word 0x"), printHex((int)*(w->name)), print(" \""), print(w->name+1), print("\" ");
      if(ip) print(" at IP 0x"), printHex(ip);
    }
  } else if( ! ( T->state & CMPLING ) ) print( " OK" );
  flush();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FVM::readPerChr () { // input char from serial port
  if ( T->state & READING == 0 ) return; // not in reading chars
  if ( ! AVAILABLE() ) return;     // serial port not ready
  char c = READ(); // read a char from serial port
//if ( c == '\b' ) {              // back space
//  if ( T->iEnd > T->tib ) print("\b \b"), T->iEnd--; // erase last character
//  return;   
//}
  if ( c == '\r' ) { *(T->iEnd) = 0; // append 0 as end of input (for carriage return)
    evalScript(T->tib); return;        // interpret tib
  }
  if ( T->iEnd >= T->tib+bytesForTib-1 ) { abort( 9, "tib full of TIB_SIZE" ); return; }
  *(T->iEnd++) = c;          // append c into tib
//print(c); // echo c
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t times=0;
void FVM::update(){
	extern void fvmHandleClient();
	fvmHandleClient();
	if(times++ == 0) showTime(), print(" first run F.update()");
	readPerChr(), evalPerTkn(), callPerWrd(); }
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FVM::init( long baud ){
  init( baud, NULL, NULL ); // no wifi communication
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//String html;
String ssid, pswd;
void FVM::init( long baud, char* _ssid_, char* _pswd_ ){
  ssid = _ssid_, pswd = _pswd_;
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
  Serial.begin( baud );
  sbEnd=SB; T = createTask(); *T->tob = 0; T->html = 0; T->err = 0; T->state = READING;
  print("\nESP32 Chip id = "), printHex(ESP.getEfuseMac()), cr();
  showTime(), print(" run F.init()"), cr();
  
  delay(100);
  T->DP = T->DS-1;           // clearing data   stack
  T->RP = T->RS-1;           // clearing return stack
  T->tracing = 0;
  
  extern char* word_set_logo;// defined in another .cpp file
  print("/////////////////////////////////////////////////\n");
  print(                     fvm_logo						 );
  print(                   word_set_logo					 );
  print("//  derek@wifiboy.org & samsuanchen@gmail.com  //\n");
  print("/////////////////////////////////////////////////\n");
  extern Word* word_set;     // defined in another .cpp file
  vocInit(word_set);
  readString();
  if(ssid != NULL && pswd != NULL ) {
	  extern void wifi();
	  wifi();
  }
//T->tracing = 1;
  extern void xxx0();
  xxx0();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////