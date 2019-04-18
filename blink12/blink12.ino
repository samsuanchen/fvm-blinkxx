// blink12.ino having 3 control variable to test
#define LED_BUILTIN 16                                  // for WIFIBOY
int  timeToChange = 1000;                               // time to change
int levelToChange = HIGH;                               // level to change
int  led          = LED_BUILTIN;                        // set GPIO 16 as led pin number
int  delayHIGH    = 1000;                               // keep 1000 ms for led pin level HIGH
int  delayLOW     = 1000;                               // keep 1000 ms for led pin level LOW
#include <fvm.h>                                        // ##### 1.1. load FVM class, the Forth virtual machine
#include <fvm_7Wordset.h>                               // ##### 1.2. load wordset for FVM
FVM F;                                                  // ##### 1.3. define F as an instence of FVM
void setup() {
  F.init( 115200 );                                     // ##### 3.1. in setup(), initialize F
  F.newVariable( "delayHIGH", &delayHIGH );             // ##### 4.1. add address as new constant word delayHIGH in F
  F.newVariable( "delayLOW" , &delayLOW  );             // ##### 4.2. add address as new constant word delayLOW  in F
  F.newVariable( "led"      , &led       );             // ##### 4.3. add address as new constant word led       in F
  pinMode(led, OUTPUT);                                 // set led pin as output level become LOW)
}
void loop() {
  if( millis() < timeToChange ) return;                 // wait until time to change
  digitalWrite( led, levelToChange );                   // set led pin level
  timeToChange += levelToChange ? delayHIGH : delayLOW; // set next time to change
  levelToChange = HIGH - levelToChange;                 // set next level to change
}
