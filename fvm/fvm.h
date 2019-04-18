// fvm.h
#ifndef _FVM_H_
#define _FVM_H_
#define fvm_logo "//  fvm - Forth Virtual Machine 1.0  20190417  //\n"
#include <arduino.h>
//////////////////////////////////////////////////////////////////////////////////////////////
#define LED_BUILTIN 16
// Serial IO ////////////////////////////////////////////////////////////////////////////////////////////
#define PRINTF	  Serial.printf    // formated print
#define PRINT     Serial.print     // print given object
#define AVAILABLE Serial.available // check if available to read
#define READ	  Serial.read      // read ascii code of available char
#define WRITE	  Serial.print     // print char of given ascii code
// running state
#define READING 1 // reading to terminal input buffer char by char.
#define PARSING 2 // parsing and evaluating from script token by token.
#define CMPLING 4 // compiling to word-list of forth colon definition word by word.
// word flag
#define IMMED       0x8000 // immediate word
#define COMPO       0x4000 // compile-only word
#define IMMED_COMPO 0xc000 // immediate compile-only word
#define HIDEN       0x2000 // hidden word (searched but not seen)
#define COMPO_HIDEN 0x6000 // hidden compile-only word
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define TIB_SIZE  2048            // default size of terminal input buffer to read characters
#define TOB_SIZE   256            // default size of terminal output buffer to print characters
#define TMP_SIZE   256            // default size of temporary buffer to parse token or to convert integer to number string
#define SB_SIZE   2048            // default size of string buffer to keep all unique strings
#define DS_SIZE     16            // default depth limit of data stack (number of 32-bit cells)
#define CS_SIZE    256            // default number of 32-bit cells as compile space for word list of a forth colon definition
#define RS_SIZE     16            // default depth limit of return stack (number of 32-bit cells)
#define CONSOLE_WIDTH 80          // console output length limit (used by the forth word "words")
#define SL_LIMIT   254            // string length limit (number of characters as leading byte, 0 as trailing byte)
#define SB_LIMIT  (SB+SB_SIZE-1)  // string buffer limit
#define DS_LIMIT  (DS+DS_SIZE-1)  // data stack limit
#define CS_LIMIT  (CS+CS_SIZE-1)  // compile space limit
#define RS_LIMIT  (RS+RS_SIZE-1)  // return stack limit
/////////////////////////////////////////////////////////////////////////////////////////////////////////
union P {                 // use 32-bit cell to hold any one of the following:
  int            con    ; // integer number
  char         * mne    ; // primitive function name
  struct Word ** wpl    ; // colon word list
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
union X {                 // use 32-bit cell to hold a floating number, an integer number, or an address
  float          f      ; // floating number
  int            i      ; // integer number
  char*			 s		; // zStr
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*FuncP)() ; // forth word code pointer type
/////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Word {     // the forth word type
  struct Word * link    ; // link to previous word
  uint16_t      id      ; // word id
  uint16_t      flag    ; // IMMED 1 immediate, COMPO 2 compileOnly, HIDEN 3 hidden,
  char        * name    ; // nStr (leading byte is name len) as the name of a forth word
  FuncP         code    ; // function code to execute
  P				p       ; // parameter field ( integer number, primitive function name, or colon word list
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Voc {      // the forth vocaburary type
  Word     * first      ; // the first forth word
  Word     * predefined ; // the last foth word predefined
  Word     * firstNew   ; // the first forth word not predefined
  Word     * context    ; // the last forth word in dictionary
  uint16_t   nWord      ; // number of all forth words in dictionary
  uint16_t   lastId     ; // word ID of the last word added into dictionary
};
typedef struct Task {     // the forth task
//----------------------------------------------------------------------------------------------------
  char     * tob        ; // terminal output buffer to print out message
  char     * tib        ; // terminal input buffer to read forth script
  int      * DS         ; // data stack
  int      * RS         ; // return stack
  Word    ** CS         ; // temporary colon word-list
//----------------------------------------------------------------------------------------------------
  char     * oEnd       ; // end of terminal output buffer
  char     * oLmt       ; // limit of terminal output buffer
  char     * iEnd       ; // end of terminal input buffer
  char     * pBgn       ; // forth script (zStr) to parse and eval
  char     * pEnd       ; // point to forth script remain (zStr)
  char     * tokenAt    ; // token found in forth script
  char     * tokenEnd   ; // end of token
  char     * hld        ; // addr to save each digit while converting number to string
//----------------------------------------------------------------------------------------------------
  Word     * context    ; // the last forth word defined in vocabulary.
  Word     * W          ; // running forth word.
  int        base=10    ; // number input/output coversion base
  uint32_t waitMsUntil=0; // wait until specified time in ms.
  int      * DP         ; // top of data stack
//----------------------------------------------------------------------------------------------------
  Word    ** IP         ; // point to next cell of colon word-list
  int      * RP         ; // top of return stack
  Word     * last       ; // the last forth word defined (may be not added into dictionary yet)
  Word    ** CP         ; // point to next cell of temporary colon word-list at compile time
//----------------------------------------------------------------------------------------------------
  int        state      ; // READING, PARSING, CMPLING, CALLING, LOADING
  int        tracing    ; // tracing depth of calling colon type forth word
  int        err        ; // error id
  char     * errMsg		; // error message
  int        warning 	; // give warning message
  String   * html		; // web server page
  
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// class FVM;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
class FVM {
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  public:
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
    FVM () {}
    virtual ~FVM () {}
    Voc   * voc;                    // dictionary of all forth words defined.
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    Task  * T;                      // running task.
    Task  * createTask();           // createTask(TOB_SIZE,DS_SIZE,RS_SIZE,CS_SIZE);
    Task  * createTask(int tob_size, int tib_size, int DS_size, int RS_size, int CS_size);
    void	abort(int, char*);		// abort as given error id and message
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    void    dClear();               // clear data stack
    int     dTop();					// get top integer of data stack
    void    dTop(int);				// replace top of data stack by given integer
    void    dPush(int);             // push given integer on top of data stack
    int	    dPop();                 // pop integer from top of data stack
    int     dPick(int i);           // push i-th item of data stack on top (0 as the top)
    void    dRoll(int i);           // roll i-th item of data stack to top (0 as the top)
    void    dBackRoll(int i);       // back roll top of data stack to i-th (0 as the top)
    int	    dDepth();               // depth of data stack
    boolean dHasItems(int);         // check if data stack has given number of items
    boolean dHasSpace(int);         // check if data stack has space for given number of items
    boolean dIsFull();              // check if data stack is full
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    void    rClear();               // clear return stack
    void    rPush(int);             // push integer onto return stack
    int     rPop();                 // pop integer from return stack
    int     rPick(int);             // top i-th integer of return stack
    int     rDepth();               // current depth of return stack
    boolean rHasItems(int);         // check if return stack has given number of items
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    void    flush();                // flush out 																		  from terminal out buffer
    void    print (char);           // print char   (flush out if  it's '\n' or buffer full)								to terminal out buffer
    void    cr();                	// print '\n'	(flush out)																to terminal out buffer
	void	qcr(int n);				// print '\n' if strlen(T->tob)>=tobLmt-n
	void	qcr(){ qcr(0); };		// 
    void    print (char*);          // print string	(flush out for each '\n' or buffer full)								to terminal out buffer
    void    print (char*, int);     // print given number of characters														to terminal out buffer
	void    print (int, int);		// print number of given base															to terminal out buffer
	void    printZ(int, int);		// print hexadecimal number of given number of digits (with leading  zero, if needed)	to terminal out buffer
	void    print (int);			// print decimal number																	to terminal out buffer
	void    printHexZ(int, int);	// print hexadecimal number of given number of digits (with leading  zero, if needed)	to terminal out buffer
	void    printHex (int, int);	// print hexadecimal number of given number of digits (with leading space, if needed)	to terminal out buffer
	void    printHex (int);			// print hexadecimal number																to terminal out buffer
	void    printHex (char*);		// print string address in hexadecimal													to terminal out buffer
	void    printHex (Word*);		// print forth word address in hexadecimal												to terminal out buffer
	void    printHex (Word**);      // print pointer of forth word address in hexadecimal									to terminal out buffer
    void    dotS();                 // show data stack
    void    dotId(Word*);           // show word's ID and name
    void    showStacks();           // show return stack and data stack
    void    dot(int);               // print integer.
    void    dotR(int i,int n,char); // print i in n-char wide (fill leading char '0' or ' ').
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    char	toDigit(int i);         			// convert int i to char as a digit (where 0 <= i < 36)
	char  * toStr (int i, int b);				// convert int i to  base b str
	char  * toStr (int i, int b, int n, char c);// convert int i to  base b str of at least n bytes with leading c if needed
    char  * toDec (int i);						// convert int i to decimal str
    char  * toDec (int i, int n); 				// convert int i to decimal str of of at least n bytes
    char  * toDecZ(int i, int n); 				// convert int i to decimal str of at least n bytes
    char  * toHex (int i);						// convert int i to		hex str.
    char  * toHex (int i, int n);				// convert int i to		hex str of at least n bytes with leading ' ' if needed
    char  * toHexZ(int i, int n);				// convert int i to 	hex str of at least n bytes with leading '0' if needed
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    boolean isFloatingNumber(char*);// return 1 if given string is a valid floating number; return 0 otherwise.
    char  * hexPrefix(char*);       // return remain string if prefix 0x or $; return 0 otherwise.
    int     toNumber(char*);        // convert token as a number to integer (or float).
    boolean isNAN();                // return 1 if token converted is not a number; return 0 otherwise.
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    void    cpInit();               // initializing word list to compile.
    void    compile(Word*);         // compile w into word list.
    Word ** cpClone();              // make a copy of word list as the colon definition of new forth word.
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    Word  * createWord(uint16_t flag, uint16_t id, char*name, FuncP code, int data);// create forth word
    void    forgetWord(char*name);
	Word  * newPrimitive(char*name, FuncP code, char*codeName);
    Word  * newPrimitive(char*,FuncP);// create forth primitive word of given name and given code.
    Word  * newConstant(char*,int); // create forth constant word of given name and value.
    Word  * newVariable(char*,int*);// create forth constant word of given name and value.
    bool    isWord(Word*);          // if given object is a forth word in vocabulary, return 1, else 0.
    void    showWordType(Word*);    // show type of given foth word
    void    vocInit(Word*);         // link given forth word as the last word in vocabulary
    Word  * vocSearch(char*);       // search the forth word of given name in vocabulary
    void    vocAdd(Word*);          // add given forth word into vocabulary
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    bool    isConstantType(Word*);  // check if given forth word is constant type then return 1; otherwise return 0.
    bool    isValueType(Word*);     // check if given forth word is value type then return 1; otherwise return 0.
    bool    isVariableType(Word*);  // check if given forth word is vareable type then return 1; otherwise return 0.
    bool    isColonType(Word*);     // check if given forth word is colon type then return 1; otherwise return 0.
    bool    isPrimitiveType(Word*); // check if given forth word is primitive type then return 1; otherwise return 0.
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    void    words(char*);           // show all forth word names including given substring
    void    dump(int*, int);        // dump cells at given address
    void    see(Word *);            // see insight of given forth word
//  void    mountFFS();             // mount flash files system
//  void    fileSystemTest();
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    void    ms(int);                // wait given number of milli seconds
    void    showTime();             // show current time in format hh:mm:ss.nnn
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    void    init(long);      		// setup baud rate to communicate via serial port
    void    init(long,char*,char*);	// setup baud rate to communicate via serial port and cpu
    void    update();               // run FVM
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    void    callColWrd();     	    // setup to call words from the word list of working colon type word
    void    callPerWrd();           // call word by word from the word list of working colon type word
    void    ipPush();               // push working colon type word and IP to return stack
    void    ipPop();                // pop IP and working colon type word from return stack
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    void    evalScript(char*);        // evaluate given script
    void    evalPerTkn();           // parse and evaluate next token in tib
    char  * uniqueNStr(char*);      // create a unique NStr for given token
	char  * parseToken(char);       // parse token by given delimiter (for example, ' ', '"', or ')')
    void    evalToken(char*);	    // evalate given token
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    void    readString();             // setup to read chars into tib
    void    readPerChr();			// read char by char to tib until '\r'
//----------------------------------------------------------------------------------------------------
//  File    curDir;
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
	int		bytesForTask;
	int		bytesForTob;
	int		bytesForTib;
	int		bytesForDS;
	int		bytesForRS;
	int		bytesForCS;
	int		bytesTotal;
  // all the following are used to compile or decompile forth words
    int     i_doCon, i_doVal, i_doVar, i_doCol, i_ret, i_doNext, i_doLit, i_zbran, i_bran, i_compile;
    Word  * w_doCon, * w_doVal, * w_doVar, * w_doCol,
          * w_ret, * w_doFor, * w_doNext, * w_doLit, * w_doStr, * w_doIf, * w_doElse, * w_doThen,
          * w_doBegin, * w_doAgain, * w_doUntil, * w_doWhile, * w_doRepeat, * w_compile, * redefined=0;
    int     needExtraCell[5]={ i_doLit, i_compile, i_doNext, i_zbran, i_bran };
    char    tmp[TMP_SIZE];          // buffer used in parseToken() and toStr() 
    char  * tmpLimit=tmp+TMP_SIZE-1;// address to hold the last '\0' of tmp
    int     hint=1; 				// hint message
    int		tobLmt=80;				// used if need to define ?cr
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
  private:
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
    char    SB[SB_SIZE];             // string buffer to hold all unique srings
    char  * sbEnd=SB;                // string buffer end
    int     lineIndex=0;             // index of input line
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    boolean isWhiteSpace(char c);   // check if c is white space
    char  * remain;                 // remain string of number converted token.
};
//////////////////////////////////////////////////////////////////////////
#define LAST 0
#endif _FVM_H_