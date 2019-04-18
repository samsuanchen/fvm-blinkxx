// fvm_6Wordset.h
#ifndef WORD_SET_H
#define WORD_SET_H
//                    "/////////////////////////////////////////////////"
char* word_set_logo = "//      fvm  6-word wordset 1.0  20190408      //\n";
//                    "/////////////////////////////////////////////////"
#include <fvm.h>
extern FVM F;
#define LAST 0
#define WORD( name ) (Word*)&W ## name
#define CONST( id, flag, symbol, name, value ) const Word W ## name = {LAST, id, flag, symbol, _doCon, (int)value}
#define PRIMI( id, flag, symbol, name, func ) const Word W ## name = {LAST, id, flag, symbol, func, (int)#func}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// W000 (con) ( -- a ) Get the address of constant type handler _doCon.
static void _doCon(){ F.dPush( F.T->W->p.con ); } // push P field content of the running forth word to stack.
CONST( 0x000, HIDEN, "\x05" "(con)", _doCon, _doCon );
#define LAST WORD( _doCon )
//////////////////////////////////////////////////////////////////////////
// W023 ! ( n addr -- ) store 32-bit number into given memory address.
static void _store () { int *a=(int*)F.dPop(); *a=F.dPop(); }
PRIMI( 0x023, 0, "\x01" "!", _store, _store );
#define LAST WORD( _store ) 
//////////////////////////////////////////////////////////////////////////
// W508 output ( pin -- ) set pin mode as OUTPUT
static void _output () { uint8_t pin=F.dPop(); pinMode(pin,OUTPUT); }
PRIMI( 0x508, 0, "\x06" "output", _output, _output );
#define LAST WORD( _output ) 
//////////////////////////////////////////////////////////////////////////
// W50a high ( pin -- ) set pin to level HIGH
static void _pinOut () { uint8_t level=F.dPop(), pin=F.dPop();
//F.print(" digitalWrite(0x"), F.printHex(pin), F.print(", 0x"), F.printHex(level), F.print(") @ "), F.showTime(), F.cr();
  digitalWrite(pin, level); }
static void _high () { F.dPush(HIGH), _pinOut(); }
PRIMI( 0x50a, 0, "\x04" "high", _high, _high );
#define LAST WORD( _high ) 
//////////////////////////////////////////////////////////////////////////
// W50b low ( pin -- ) set pin to level LOW
static void _low () { F.dPush(LOW), _pinOut(); }
PRIMI( 0x50b, 0, "\x03" "low", _low, _low );
#define LAST WORD( _low ) 
//////////////////////////////////////////////////////////////////////////
// W50d input ( pin -- ) set pin mode as INPUT
static void _input(){
  uint8_t pin=F.dPop();
  pinMode(pin,INPUT); }
PRIMI( 0x50d, 0, "\x05" "input", _input, _input );
#define LAST WORD( _input ) 
/////////////////////////////////////////////////////////////////////////////////////////////////////////
const Word* word_set=LAST;
#endif WORD_SET_H
