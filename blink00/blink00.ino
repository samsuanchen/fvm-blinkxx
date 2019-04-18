  // blink00.ino the Arduino basic blink
  #define LED_BUILTIN 16            // for WIFIBOY
  
  
  void setup() {
    pinMode(LED_BUILTIN, OUTPUT);   // set led pin as output
  }
  void loop() {
    digitalWrite(LED_BUILTIN, HIGH);// set led pin level as HIGH
    delay(1000);                    // wait a second
    digitalWrite(LED_BUILTIN, LOW); // set led pin level as LOW
    delay(1000);                    // wait a second
  }
