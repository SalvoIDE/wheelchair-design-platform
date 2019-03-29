/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/*
    Please note the long strings of data sent mean the *RTS* pin is
    required with UART to slow down data sent to the Bluefruit LE!
*/
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#include "BluefruitConfig.h"

// LED error flag
#define LED_PIN 13

// Create the Bluefruit object for Feather 32u4
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// BNO settings
#define BNO055_SAMPLERATE_DELAY_MS (200)
Adafruit_BNO055 bno = Adafruit_BNO055(55);

// GATT service information
int32_t imuServiceId;
int32_t proximityCharId;

// A small helper
void error(const __FlashStringHelper*err) {
  if (Serial.available()) {
    Serial.println(err);
  }
  // In any case, turn on the LED to signal the error
  analogWrite(LED_PIN, HIGH);
  while (1);
}

// Initializes BNO055 sensor
void initSensor(void) {
  if(!bno.begin()) {
    error(F("No BNO055 detected. Check your wiring or I2C ADDR!"));
  }
  delay(1000);
  bno.setExtCrystalUse(true);
}

#define PROXIMITY_PIN  A0                       // Setting up pin to receive voltage from IR

int value, prev_value = - 10000;         // int values (read from analog port, both the current and the previous)
int deviation = 0;                       // setting the minimum deviation between the measurements (0 by default)
                                         // up to 512 (although that is pretty useless)

// Sets up the HW an the BLE module (this function is called
// automatically on startup)
void setup(void) {
  delay(500);
  boolean success;

  pinMode(PROXIMITY_PIN, INPUT);                // setting pinmode to read analog value 

  deviation = 10;

  // Set LED error flag

  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, LOW);
  Serial.begin(115200);

  // Initialise the module
  if ( !ble.begin(VERBOSE_MODE) ) {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring."));
  }

   // Setup the BNO055 sensor
  initSensor();

  // Perform a factory reset to make sure everything is in a known state
  if (! ble.factoryReset() ){
       error(F("Couldn't factory reset."));
  }

  // Disable command echo from Bluefruit
  ble.echo(false);

  // Print Bluefruit information
  ble.info();
  ble.verbose(true);

  // Change the device name to fit its purpose HERE WE PUT ZOOFRUIT
  if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Zoofruit")) ) {
    error(F("Could not set device name."));
  }

  // Add the IMU Service definition
  success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID128=00-11-00-11-44-55-66-77-88-99-AA-BB-CC-DD-EE-FF"), &imuServiceId);
  if (! success) {
    error(F("Could not add Orientation service."));
  }

  // Add the Proximity characteristic
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID128=02-11-88-33-44-55-66-77-88-99-AA-BB-CC-DD-EE-FF,PROPERTIES=0x10,MIN_LEN=1,MAX_LEN=17,VALUE=\"\""), &proximityCharId);
  if (! success) {
    error(F("Could not add Proximity characteristic."));
  }

  // Add the Orientation Service to the advertising data
  // (needed for Nordic apps to detect the service)
  ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-05-02-0d-18-0a-18") );

  // Reset the device for the new service setting changes to take effect
  ble.reset();
}

void proximity() {
  // Get Euler angle data

  value = analogRead(PROXIMITY_PIN);       // reading our analog voltage, careful we only have 10 bit resolution so each
                                    // measurement step is only 5V ÷ 1024, so our result will be 0 - 1023

  // if value is within the range of [ previous - σ , previous + σ], ignore it (if value is relatively the same)
  // this will help with having data ocuppy your buffer that is not a significant deviation.
  if( value >= (prev_value - deviation) && value <= (prev_value + deviation) )
    return;

  prev_value = value;             // Here we have the previous saved variable.


  
  // Command is sent when \n (\r) or println is called
  // AT+GATTCHAR=CharacteristicID,value
  ble.print( F("AT+GATTCHAR=") );
  ble.print( proximityCharId );
  ble.print( F(",") );
  ble.print(String(value));
}


void loop(void) {

  proximity();

  // Check if command executed OK
  if ( !ble.waitForOK() ) {
    error(F("Failed to get response!"));
  }

  // Delay before next measurement update
  delay(BNO055_SAMPLERATE_DELAY_MS);
}
