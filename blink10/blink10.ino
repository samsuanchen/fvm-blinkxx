  // blink10.ino not using delay() to keep led pin level LOW/HIGH
  #define LED_BUILTIN 16                      // GPIO number for WIFIBOY led builtin
  int  timeToChange = 1000;                   // time to change
  int levelToChange = HIGH;                   // level to change
  void setup() {
    pinMode(LED_BUILTIN, OUTPUT);             // set led pin as output level become LOW)
  }
  void loop() {
    if( millis() < timeToChange ) return;     // wait until time to change
    digitalWrite(LED_BUILTIN, levelToChange );// set led pin level
    timeToChange += 1000; 				      // set next time to change
    levelToChange = HIGH - levelToChange;     // set next level to change
  }
