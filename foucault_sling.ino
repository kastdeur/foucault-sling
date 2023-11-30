/*
Foucault Sling (IR interrupt Blinker)
*/

const int irPin = 2;

int irState =  HIGH; // It is pulled low on trigger
unsigned long curMills = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(irPin, INPUT);
}

void loop() {

  while ( millis() - curMills < 2000 )
  {
    delay(100);
  }

  // new loop
  curMills = millis();
  irState = digitalRead(irPin);

  digitalWrite(LED_BUILTIN, irState ? LOW : HIGH );
}