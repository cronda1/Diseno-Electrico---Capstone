


void setup() {
Serial.begin(9600);
pinMode(8, INPUT); // Setup for leads off detection LO +
pinMode(9, INPUT); // Setup for leads off detection LO -
}
void loop() {
Serial.println(analogRead(25));

delay(1);
}
