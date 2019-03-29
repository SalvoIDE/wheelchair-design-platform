#include <Adafruit_NeoPixel.h>

#define PIN 8 
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);
//WHICH OF THE TWO SHOULD WE USE?
#define REDPIN 5
#define GREENPIN 6
#define BLUEPIN 3
#define FADESPEED 5     // make this higher to slow down

#define VIBRATION_PIN 10 

boolean tired = false ; 

void vibration_enabled() {
}

void led_yellow() {
int r, g, b;
        // fade from red to yellow
  for (g = 0; g < 256; g++) { 
    analogWrite(GREENPIN, g);
    delay(FADESPEED);
    }
}

void led_green() {
 // fade from green to teal
 int r, g, b;
  for (b = 0; b < 256; b++) { 
    analogWrite(BLUEPIN, b);
    delay(FADESPEED);

    //OR

//    strip.setPixelColor(n, red, green, blue);
//different way of showing color: 
//  LED_controller.setPixelColor( 0, 0xFFFFFF);
      strip.setPixelColor(11, 255, 0, 255);
      strip.show();
    // how can we make 
     }
//how do we make the program start running with a green light?
}

void setup() {
  // put your setup code here, to run once:
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(VIBRATION_PIN, OUTPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
 Serial.begin(9600); 
 Serial.println("Green light for starting!");
  }


void loop() {
  // put your main code here, to run repeatedly:
char command = Serial.read();
  if (command == '1') {
    Serial.println("User tired - yellow light...");
    tired = true;
  } else if (command == '0') {
    Serial.println("User not tired - green light...");
    tired = false;
   // tired = false;
    led_green();  // ***** is this good here?
// do i make two functions for differnt led colors or how do you integrate this into one function like just changing the color?
  }

 
  if (tired) {
    vibration_enabled();
    led_yellow();

    delay(300);  //can the delay go inside here so that light lasts this amount of time?
    vibration_enabled(); //at the end of this time we also give a vibration when the thing turns to green again
    }
// 
//  if (not_tired) {
//    led_green();
//    analogWrite(LED_PIN, i); // here find code for giving it color!
//     }
//     
  delay (20);
}
    
