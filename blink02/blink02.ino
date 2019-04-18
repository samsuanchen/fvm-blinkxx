// blink02.ino having 3 control variables to test
#define LED_BUILTIN 16           // for WIFIBOY
int  led          = LED_BUILTIN; // led pin GPIO number
int  delayHIGH    = 1000;        // delay period keeping led pin level HIGH
int  delayLOW     = 1000;        // delay period keeping led pin level LOW
#include <fvm.h>                                  // ##### 1.1. load FVM class, the Forth virtual machine
#include <fvm_6Wordset.h>                         // ##### 1.2. load wordset for FVM
FVM F;                                            // ##### 1.3. define F as an instence of FVM
void setup() {
  F.init( 115200 );                               // ##### 3.1. in setup(), initialize F 
  F.newVariable( "delayHIGH", &delayHIGH );       // ##### 4.1. add address as new constant word delayHIGH in F
  F.newVariable( "delayLOW" , &delayLOW  );       // ##### 4.2. add address as new constant word delayLOW  in F
  F.newVariable( "led"      , &led       );       // ##### 4.3. add address as new constant word led       in F
  pinMode(led, OUTPUT);          // set led pin as output
}
void loop() {
  digitalWrite(led, HIGH);       // set led pin level as HIGH
  delay(delayHIGH);              // wait a second
  digitalWrite(led, LOW);        // set led pin level as LOW
  delay(delayLOW);               // wait a second
}
