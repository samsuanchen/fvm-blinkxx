// blink03.ino having new words to execute
#define LED_BUILTIN 16          // for WIFIBOY
int led          = LED_BUILTIN; // led pin GPIO number
int delayHIGH    = 1000;        // delay period keep keeping led pin level HIGH
int delayLOW     = 1000;        // delay period keep keeping led pin level LOW
#include <fvm.h>                                  // ##### 1.1. load FVM class, the Forth virtual machine
#include <fvm_0Wordset.h>                         // ##### 1.2. load wordset for FVM
FVM F;                                            // ##### 1.3. define F as an instence of FVM
void setDelayHIGH() { delayHIGH=F.dPop(); }       // ##### 2.1. define the function setDelayHIGH
void setDelayLOW()  { delayLOW =F.dPop(); }       // ##### 2.2. define the function setDelayLOW
void setLed()       { led      =F.dPop(); }       // ##### 2.3. define the function setLed
void output() { pinMode(F.dPop(), OUTPUT); }      // ##### 2.4. define the function output
void input()  { pinMode(F.dPop(),  INPUT); }      // ##### 2.5. define the function input
void high()  { digitalWrite(F.dPop(), HIGH); }    // ##### 2.6. define the function high
void low()   { digitalWrite(F.dPop(),  LOW); }    // ##### 2.7. define the function low
void setup() {
  F.init( 115200 );                           // ##### 3.1. in setup(), initialize F 
  F.newPrimitive( "setDelayHIGH", setDelayHIGH ); // ##### 4.1. add new primitive word setDelayHIGH in F
  F.newPrimitive( "setDelayLOW",  setDelayLOW  ); // ##### 4.2. add new primitive word setDelayLOW  in F
  F.newPrimitive( "setLed"     ,  setLed       ); // ##### 4.3. add new primitive word setLed       in F
  F.newPrimitive( "output"     ,  output       ); // ##### 4.3. add new primitive word output       in F
  F.newPrimitive(  "input"     ,   input       ); // ##### 4.3. add new primitive word  input       in F
  F.newPrimitive(   "high"     ,    high       ); // ##### 4.3. add new primitive word   high       in F
  F.newPrimitive(    "low"     ,     low       ); // ##### 4.3. add new primitive word    low       in F
  pinMode(led, OUTPUT);     // set led pin as output
}
void loop() {
  digitalWrite(led, HIGH);     // set led pin level as HIGH
  delay(delayHIGH);            // wait a second
  digitalWrite(led, LOW);      // set led pin level as LOW
  delay(delayLOW);             // wait a second
}
