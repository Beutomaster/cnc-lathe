
#define MAX_PIN 65

int i=0;

void setup() {                
  // initialize all pins as output
  for (i=0; i<MAX_PIN; i++) {
    pinMode(i, OUTPUT);
  }
}

void loop() {
  for (i=0; i<MAX_PIN; i++) {
    digitalWrite(i, HIGH);   // set all LEDs on
  }
    delay(1000);              // wait for a second
  for (i=0; i<MAX_PIN; i++) {
    digitalWrite(i, LOW);    // set all LEDs off
  }
    delay(1000);              // wait for a second
}
