
#define LED_PIN 2 // Careful, here we have to use a pin that can be used for pwm.

 int i = 20; // Our counter for PWM, we declare it globally
// so it lasts for the duration of the entire program
boolean direction_down = 20; // boolean so we know when to start to invert direction of the counter
boolean led_enabled = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600); // setting baud speed for Serial (a baud is a pulse)
  Serial.println("Lets start our pulsing LED example!");
}


void led_pattern() {
  if( i == 80)
    direction_down = true;
  else if( i == 20)
    direction_down = false;

  if(direction_down)
    i--;
  else
    i++;

    Serial.print("Led intensity (0 to 255):  ");
    Serial.println(i);
    analogWrite(LED_PIN, i);
}


void loop() {
  char command = Serial.read();
  if (command == '1') {
    Serial.println("Turning on Led...");
    led_enabled = true;
  } else if (command == '0') {
    Serial.println("Turning off Led...");
    led_enabled = false;
    analogWrite(LED_PIN, 0);
  }
  if (led_enabled) {
    led_pattern();
    analogWrite(LED_PIN, i);
  }
  delay(50);
}
