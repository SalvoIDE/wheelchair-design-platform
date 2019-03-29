#include <Adafruit_NeoPixel.h> // Necessary Library include


#define LED_PIN 6 // Defining the pin of the arduino that sends the data stream.

Adafruit_NeoPixel strip = Adafruit_NeoPixel( 1, LED_PIN, NEO_RGB + NEO_KHZ800);
// This is a class that will control your NeoPixel LEDs
//( NUMBER OF LEDS Connected, PIN NUMBER, LED TYPE, IN OUR CASE NEO_RGB + NEO_KHZ800),
// NEO_KHZ800 just mentions the 800 KHz support, no need to change for most cases.
uint8_t R = 0, G = 0, B = 0; // Unsigned integer with 8 bits
uint32_t counter = 0; // 32 bits unsigned integer, we only need 24 to go through all the colors
//
//void vibration_enabled() {
//}
//
//void led_yellow() {
//  strip.setPixelColor( 1, strip.Color( 150, 0, 0 ) ); // yellow?!
//  strip.show();
//  Serial.println("Yellow light!");
//
//}
//
//void led_green() {
//  strip.setPixelColor( 1, strip.Color( 0, 150, 0 ) ); // Green!
//  strip.show();
//  Serial.println("Green light!");
//
//}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //Set serial to 9600 baud
  pinMode(LED_PIN, OUTPUT);
  strip.begin(); // We're starting up the library
  strip.setPixelColor( 1, strip.Color( 0, 0, 150 ) ); // Green!
}

void loop() {

   strip.setPixelColor( 1, strip.Color( 150, 0, 0 ) ); // yellow?!
   strip.show();
   Serial.println("Yellow light!");
   delay(700);
   strip.setPixelColor( 1, strip.Color( 0, 150, 0 ) ); // Green!
   strip.show();
   Serial.println("Green light!");
   delay(700);

//  led_yellow();
//  delay(500);
//  led_green();
//  delay(500);


//  if(counter >= 0xFFFFFF) counter = 0; // once we go over the the maximum value 255 255 255, we go back to 0
//  counter+= 10000;
//  counter&= 0xFFFFFF; // bit mask to only keep the first 24bits

}
