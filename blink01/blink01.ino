  // blink01.ino having F as an instence of FVM to play
  #define LED_BUILTIN 16             // for WIFIBOY
  #include <fvm.h>                              // ##### 1.1. load FVM class, the Forth virtual machine
  #include <fvm_wifiboy_libWordset.h>           // ##### 1.2. load wordset for FVM
  FVM F;                                        // ##### 1.3. define F as an instence of FVM
  void setup() {
    F.init( 115200 );                           // ##### 3.1. in setup(), initialize F with serial monitor of given baud rate
    pinMode(LED_BUILTIN, OUTPUT);    // set led pin mode as output
  }
  void loop() {
    digitalWrite(LED_BUILTIN, HIGH); // set led pin level as HIGH
    delay(1000);                     // wait a second
    digitalWrite(LED_BUILTIN, LOW);  // set led pin level as LOW
    delay(1000);                     // wait a second
  }
