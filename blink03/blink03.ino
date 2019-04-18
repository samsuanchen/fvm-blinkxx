/* blink03.ino
  Open Arduino IDE Serial Monitor, from input box we may try the following one by one:
  50 setDelayLOW      ( Let the led become short flash per second. 讓燈每秒短暫閃亮 )
  50 setDelayHIGH     ( Let the led become flash quickly. 讓燈快速閃亮 )
  25 setLed 25 output 17 output 17 high ( Let the buzzer hum. 讓蜂鳴器滴答作響 )
  17 low              ( Let the buzzer off. 讓蜂鳴器靜音 )
*/
#define LED_BUILTIN 16
int  led          = LED_BUILTIN;
int  delayHIGH    = 1000;
int  delayLOW     = 1000;
#include <fvm.h>                                          // ##### 1.1. load FVM class, the Forth virtual machine
#include <fvm_0Wordset.h>                                 // ##### 1.2. no predefined wordset for FVM
FVM F;                                                    // ##### 1.3. define F as an instence of FVM
void setDelayHIGH() { delayHIGH = F.dPop(); }             // ##### 2.1. define new function setDelayHIGH
void setDelayLOW()  { delayLOW  = F.dPop(); }             // ##### 2.2. define new function setDelayLOW
void setLed()       { led       = F.dPop(); }             // ##### 2.3. define new function setLed
void output()       { pinMode( F.dPop(), OUTPUT); }       // ##### 2.4. define new function output
void  input()       { pinMode( F.dPop(),  INPUT); }       // ##### 2.5. define new function  input
void   high()       { digitalWrite( F.dPop(), HIGH); }    // ##### 2.6. define new function   high
void    low()       { digitalWrite( F.dPop(), LOW); }     // ##### 2.7. define new function    low
void setup() { // the setup function runs once when you press reset or power the board
  F.init( 115200 );                                       // ##### 3.1. initialize F in setup function
  F.newPrimitive( "setDelayHIGH", setDelayHIGH );         // ##### 4.1. add new primitive word setDelayHIGH in F
  F.newPrimitive( "setDelayLOW" , setDelayLOW  );         // ##### 4.2. add new primitive word setDelayLOW  in F
  F.newPrimitive( "setLed"      , setLed       );         // ##### 4.3. add new primitive word setLed       in F
  F.newPrimitive( "output"      , output       );         // ##### 4.4. add new primitive word output       in F
  F.newPrimitive(  "input"      ,  input       );         // ##### 4.5. add new primitive word  input       in F
  F.newPrimitive(   "high"      ,   high       );         // ##### 4.6. add new primitive word   high       in F
  F.newPrimitive(    "low"      ,    low       );         // ##### 4.7. add new primitive word    low       in F
  pinMode(led        , OUTPUT);      // initialize digital pin led         as output.
}
void loop() { // the loop function runs over and over again forever
  digitalWrite(led        , HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(delayHIGH);                  // wait for a second
  digitalWrite(led        , LOW);    // turn the LED off by making the voltage LOW
  delay(delayLOW );                  // wait for a second
}
