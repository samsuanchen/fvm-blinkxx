// blink11.ino having F as an instence of FVM to play
#define LED_BUILTIN 16            // for WIFIBOY
int  timeToChange = 1000;         // time to change
int levelToChange = HIGH;         // level to change
#include <fvm.h>                                        // ##### 1.1. load FVM class, the Forth virtual machine
#include <fvm_wifiboy_libWordset.h>                     // ##### 1.2. load wordset for FVM
FVM F;                                                  // ##### 1.3. define F as an instence of FVM
void setup() {
  F.init( 115200 );                           // ##### 3.1. in setup(), initialize F
  pinMode(LED_BUILTIN, OUTPUT);                           // set led pin as output level become LOW)
}
void loop() {
  if( millis() < timeToChange ) return;                   // wait until time to change
  digitalWrite( LED_BUILTIN, levelToChange );             // set led pin level
  timeToChange += 1000;             // set next time to change
  levelToChange = HIGH - levelToChange;                   // set next level to change
}
