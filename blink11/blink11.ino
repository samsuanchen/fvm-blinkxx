  // blink11.ino having F as an instence of FVM to play
  #define LED_BUILTIN 16             // for WIFIBOY
  int  timeToChange = 1000;
  int levelToChange = HIGH;
  #include <fvm.h>                              // ##### 1.1. load FVM class, the Forth virtual machine
  #include <fvm_wifiboy_libWordset.h>           // ##### 1.2. load wordset for FVM
  FVM F;                                        // ##### 1.3. define F as an instence of FVM
  void setup() {
    F.init( 115200 );                           // ##### 3.1. in setup(), initialize F and the word set
    pinMode(LED_BUILTIN, OUTPUT);                          // set led pin mode as output (pin level become LOW)
  }
  void loop() {
    if( millis() < timeToChange ) return;                   // do nothing before time to change
    digitalWrite( LED_BUILTIN, levelToChange );             // change led pin level
    timeToChange += 1000;                                   // next time to change led pin level
    levelToChange = HIGH - levelToChange;                   // next led pin level to change
  }
