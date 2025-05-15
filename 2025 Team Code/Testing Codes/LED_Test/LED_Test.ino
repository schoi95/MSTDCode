#define RED_LED   10
#define GREEN_LED 11
#define BLUE_LED  12  // Optional: test if blue still works

void setup() {
  pinMode(10, OUTPUT);
}
void loop() {
  digitalWrite(10, LOW);  // Should light RED
  delay(500);
  digitalWrite(10, HIGH);
  delay(500);
}
