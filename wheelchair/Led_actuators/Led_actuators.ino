// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define LED_PIN 2
#define VIB_PIN 3
#define PROX_PIN A0
int proximity_value = - 10000;       
//char property_id = "surf-wheel-proximity-b6f1";

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      10
bool nudged = false;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

int i = 225;
int delayval = 500; // delay for half a second
boolean tired = false;
boolean vibration = false;

void vibration_enabled() {
   Serial.println("vibrating on");
   analogWrite(VIB_PIN, i);
   delay(550);
   analogWrite(VIB_PIN, 0);
   delay(100);
   analogWrite(VIB_PIN, i);
   delay(100);
   analogWrite(VIB_PIN, 0);
   delay(200);
}


void green_led(){

//  for(int i=0;i<NUMPIXELS;i++){
//    Serial.println("User not tired - green light...");
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(0, pixels.Color(0,150,100)); // Moderately bright green color.
    pixels.setPixelColor(1, pixels.Color(0,150,100)); // Moderately bright green color.
    pixels.setPixelColor(2, pixels.Color(0,150,100)); // Moderately bright green color.
    pixels.setPixelColor(3, pixels.Color(0,150,100)); // Moderately bright green color.
    pixels.setPixelColor(4, pixels.Color(0,150,100)); // Moderately bright green color.
    pixels.setPixelColor(5, pixels.Color(0,150,100)); // Moderately bright green color.
    pixels.setPixelColor(6, pixels.Color(0,150,100)); // Moderately bright green color.
    pixels.setPixelColor(7, pixels.Color(0,150,100)); // Moderately bright green color.
    pixels.setPixelColor(8, pixels.Color(0,150,100)); // Moderately bright green color.
    pixels.setPixelColor(9, pixels.Color(0,150,100)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
  }


void yellow_led() {
//    for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
//    Serial.println("User tired - yellow light..."); 
    pixels.setPixelColor(0, pixels.Color(200,50,0)); // Moderately bright green color.
    pixels.setPixelColor(1, pixels.Color(200,50,0)); // Moderately bright green color.
    pixels.setPixelColor(2, pixels.Color(200,50,0)); // Moderately bright green color.
    pixels.setPixelColor(3, pixels.Color(200,50,0)); // Moderately bright green color.
    pixels.setPixelColor(4, pixels.Color(200,50,0)); // Moderately bright green color.
    pixels.setPixelColor(5, pixels.Color(200,50,0)); // Moderately bright green color.
    pixels.setPixelColor(6, pixels.Color(200,50,0)); // Moderately bri§ght green color.
    pixels.setPixelColor(7, pixels.Color(200,50,0)); // Moderately bright green color.
    pixels.setPixelColor(8, pixels.Color(200,50,0)); // Moderately bright green color.
    pixels.setPixelColor(9, pixels.Color(200,50,0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.


}


void setup() {
  pinMode(VIB_PIN, OUTPUT);
  analogWrite(VIB_PIN, LOW);
  pinMode(PROX_PIN, INPUT);
  Serial.println("let's start this zoo journey");
  pixels.begin(); // This initializes the NeoPixel library.
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pixels.show();
  }

void loop() {
  proximity_value = analogRead(PROX_PIN);
//  Serial.print("surf-wheel-proximity-b6f1");
//  Serial.print(",");
  Serial.print(proximity_value);
//  Serial.print(",");
//  Serial.println("");
  char command = Serial.read();
     if (command == '1') {
       tired = true;
       vibration = true;
       delay (1000);
       vibration = false;
      } else if (command == '0') {
        tired = false;
        green_led();
      }

if (tired) {
  yellow_led();
}
else {
  green_led();
  }
}
