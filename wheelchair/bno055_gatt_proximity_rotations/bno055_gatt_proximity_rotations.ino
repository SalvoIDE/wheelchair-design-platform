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

// Create the Bluefruit object for Feather 32u4
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// BNO settings
#define BNO055_SAMPLERATE_DELAY_MS (400)

Adafruit_BNO055 bno = Adafruit_BNO055(55);


// structure to store total rotations since IMU  initialized, forward and reverse
// initialized with a global variable global_rotations, this variable stores rotations
// on a particular axis, in both directions, since startup
struct Rotations {
  float forward_rotations = 0;
  float reverse_rotations = 0;
} global_rotations;

bool not_first_loop = false; // Boolean variable to stop logging of first loop
float previous_axis_value = 666;  // Initial value so we don't account for it


// GATT service information
int32_t imuServiceId;
int32_t proximityCharId;
int32_t rotationCharId;


// A small helper
void error(const __FlashStringHelper*err) {
  if (Serial.available()) {
    Serial.println(err);
  }
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
//int deviation = 0;                       // setting the minimum deviation between the measurements (0 by default)
                                         // up to 512 (although that is pretty useless)

// Sets up the HW an the BLE module (this function is called
// automatically on startup)
void setup(void) {
  delay(300);
  boolean success;

  pinMode(PROXIMITY_PIN, INPUT);                // setting pinmode to read analog value 


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

// Add the Rotation characteristic
  success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID128=06-11-87-33-44-55-66-77-88-99-AA-BB-CC-DD-EE-FF,PROPERTIES=0x10,MIN_LEN=1,MAX_LEN=12,VALUE=\"\""), &rotationCharId);
  if (! success) {
    error(F("Could not add Rotation characteristic."));
  }


  // Add the Orientation Service to the advertising data
  // (needed for Nordic apps to detect the service)
  ble.sendCommandCheckOK( F("AT+GAPSETADVDATA=02-01-06-05-02-0d-18-0a-18") );

  // Reset the device for the new service setting changes to take effect
  ble.reset();
}



bool compute_rotations(float axis, Rotations * rotations) {
  static float initial_axis_value = axis;
  // variable to store initial axis value in compute rotations - declared static so that it stores
  // this value in between function calls, but no other functions can change its value
  //Variables declared as static will only be created and initialized the first time a function is called

  float offset_rot = (axis-previous_axis_value) / 360; // offset since previous measurement, in rotations

  // so we do not account for anything in the setup phase
  if (previous_axis_value == 666) {
    offset_rot = 0;
  }

    if(offset_rot >= 0) {
    (rotations->forward_rotations) += offset_rot;
  } else {
    (rotations->reverse_rotations) += offset_rot;
  }

  // place previous axis value
  previous_axis_value = axis;

  return(true); // returns true by default, do not remove, as it helps with the initial setup.
}

void rotation() {
  /* Get a new sensor event */
  sensors_event_t event;
  bno.getEvent(&event);

  // if this is the first loop iteration, ignore position data (always zero)
  //if its second loop iteration set the starting position for your axis
  // if its another iteration, just continue computing the rotation data

  float axis_value = event.orientation.x;   // replace this with whatever axis you're tracking
  not_first_loop = (not_first_loop)?compute_rotations(axis_value, &global_rotations) : true;

  // Command is sent when \n (\r) or println is called
  // AT+GATTCHAR=CharacteristicID,value
  ble.print( F("AT+GATTCHAR=") );
//  ble.print( F(",") );
  ble.print( rotationCharId );
  ble.print( F(",") );
//  ble.print( F("Rotations" ) );
//  ble.print( F(",") );
  ble.println(String(global_rotations.forward_rotations) );

}



void proximity() {
  // Get Euler angle data

  value = analogRead(PROXIMITY_PIN);       // reading our analog voltage, careful we only have 10 bit resolution so each
                                    // measurement step is only 5V ÷ 1024, so our result will be 0 - 1023

  // if value is within the range of [ previous - σ , previous + σ], ignore it (if value is relatively the same)
//  // this will help with having data ocuppy your buffer that is not a significant deviation.
//  if( value >= (prev_value - deviation) && value <= (prev_value + deviation) )
//    return;
//
///UNCOMMENT TO IGNORE MORE


  prev_value = value;             // Here we have the previous saved variable.


  // Command is sent when \n (\r) or println is called
  // AT+GATTCHAR=CharacteristicID,value
  ble.print( F( "AT+GATTCHAR=") );
//  ble.print( F(",") );
  ble.print( proximityCharId );
  ble.print( F(",") );
//  ble.print( F("Proximity") );
//  ble.print( F(",") );
  ble.println(String(value));

}


void loop(void) {

  proximity();
  rotation();


  // Check if command executed OK
  if ( !ble.waitForOK() ) {
    error(F("Failed to get response!"));
  }

  // Delay before next measurement update
  delay(BNO055_SAMPLERATE_DELAY_MS);
}
