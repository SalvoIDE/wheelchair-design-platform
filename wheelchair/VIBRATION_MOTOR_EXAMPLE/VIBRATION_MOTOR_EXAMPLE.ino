// Lets make our vibration example, using PWM!
#define VIB_PIN 10 // Careful, here we have to use a pin that can be used for pwm.

int i = 127;       // Our counter for PWM, we declare it globally,
                   // So it lasts for the duration of the entire program.
                   // It starts in 127 since the vibration motor starts
                   // working from around 2 volts (so we start at 2.5v)
boolean increase = false;
boolean vibration_enabled = false;


void setup() {
  // put your setup code here, to run once:
  pinMode(VIB_PIN, OUTPUT);
  Serial.begin(9600); // setting baud speed for Serial (a baud is a pulse)
  Serial.println("Lets start our pulsing vibration example!");
}

void vibration_pattern() {
  if (increase) {
    i+=10;  // incrementing the power of the vibration motor
  } else {
    i-=10;
  }

  if ( i > 255) {
    increase = false;
  } else if ( i < 127) {
    increase = true;
  }
}

void loop() {
    vibration_pattern();
    delay(50);
    har command = Serial.read();
      if (command == '1') {
        Serial.println("Turning on Vibration...");
        vibration_enabled = true;
      } else if (command == '0') {
        Serial.println("Turning off Vibration...");
        vibration_enabled = false;
        analogWrite(VIB_PIN, 0);
      }
      if (vibration_enabled) {
        vibration_pattern();
        analogWrite(VIB_PIN, i);
      }
      delay(50);


  }
  // PWM takes values from 0 to 255, in our case, we want to make
  // a pulse effect, so we detect out of bounds behaviour and go to 127

  i+=10;  // incrementing the power of the vibration motor

  if( i > 255)
    i = 127;


    Serial.print("Vibration intensity (127 to 255):  ");
    Serial.println(i);
    analogWrite(VIB_PIN, i);
    delay(i*10); // in each step of pwm, we vibrate for i * 0.01 seconds

}