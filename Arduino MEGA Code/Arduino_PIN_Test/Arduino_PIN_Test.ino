
#define MAX_PIN 69

int i=0;

void setup() {                
  // initialize all pins as output
  for (i=0; i<=MAX_PIN; i++) {
    pinMode(i, OUTPUT);
  }
}

void loop() {
  //test with fast running light, because of the overall power consumption limit of 200mA for all pins
  for (i=0; i<=MAX_PIN; i++) {
    digitalWrite(i, HIGH);   // set all LEDs on
    delay(300);              // wait for a second
    digitalWrite(i, LOW);    // set all LEDs off
    delay(300);              // wait for a second
  }
}
