//
// *********************************************************************
//  This is an example for our nRF51822 based Bluefruit LE modules
//
//  Pick one up today in the adafruit shop!
//
//  Adafruit invests time and resources providing this open source code,
//  please support Adafruit and open-source hardware by purchasing
//  products from Adafruit!
//
//  MIT license, check LICENSE for more information
//  All text above, and the splash screen below must be included in
//  any redistribution
// *********************************************************************/
//
// /*
//     Please note the long strings of data sent mean the *RTS* pin is
//     required with UART to slow down data sent to the Bluefruit LE!
// */
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
#define BNO055_SAMPLERATE_DELAY_MS (200)
#define IR_PIN  A0
// WHAT IS HERE

// Creating our sensor object to handle the sensor, with initialization 12345
Adafruit_BNO055 bno = Adafruit_BNO055(55);


int value, prev_value = - 10000;         // int values (read from analog port, both the current and the previous)
int deviation = 0;                       // setting the minimum deviation between the measurements (0 by default)
///whats thisssssss?


double voltage_value, distance_value;    // Converted to Voltage
//THIS IS FROM  THE ARDUINO ..HOW CAN I ADAPT THEM HERE
double convert_to_distance( double voltage)
{
    if(voltage_value  < 0.35  || voltage_value > 2.85 ) // We will ignore values outside the range of measurement, this will happen around 20 - 150cm
    return(0);


    double a =    4498;
    double b =  -6.351;
    double c =   104.9;
    double d = -0.6928;


      return(a*exp(b*voltage) + c*exp(d*voltage));
    }



// structure to store total rotations since IMU  initialized, forward and reverse
// initialized with a global variable global_rotations, this variable stores rotations
// on a particular axis, in both directions, since startup

struct Rotations {
  float forward_rotations = 0;
  float reverse_rotations = 0;
} global_rotations;

bool not_first_loop = false; // Boolean variable to stop logging of first loop
float previous_axis_value = 666;  // Initial value so we don't account for it
bool person_behind = false;




// GATT service information
int32_t imuServiceId;
int32_t rotationCharId;
int32_t proximityCharId;

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

  // Sets up the HW an the BLE module (this function is called
  // automatically on startup)
  void setup(void) {
    delay(500);
    boolean success;


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

    // Change the device name to fit its purpose
    if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Surfing Wheelchair")) ) {
      error(F("Could not set device name."));
    }

    // Add the IMU Service definition
    success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID128=00-11-00-11-44-55-66-77-88-99-AA-BB-CC-DD-EE-FF"), &imuServiceId);
    if (! success) {
      error(F("Could not add Orientation service."));
    }

    // Add the Rotation characteristic
    success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID128=02-11-87-33-44-55-66-77-88-99-AA-BB-CC-DD-EE-FF,PROPERTIES=0x10,MIN_LEN=1,MAX_LEN=12,VALUE=\"\""), &rotationCharId);
    if (! success) {
      error(F("Could not add Rotation characteristic."));
    }



        // Add the Proximity characteristic

        // what UUID should i add here?
        success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID128=08-11-87-33-44-55-66-77-88-99-AA-BB-CC-DD-EE-FF,PROPERTIES=0x10,MIN_LEN=1,MAX_LEN=12,VALUE=\"\""), &proximityCharId);
        if (! success) {
          error(F("Could not add Proximity characteristic."));
        }


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
     ble.print( rotationCharId );
     ble.print( F(",") );
     ble.print(String(global_rotations.forward_rotations));
     ble.print( F(",") );
     ble.println(String(-global_rotations.reverse_rotations));
   }

   void proximity() {
     /* Get a new sensor event */
     sensors_event_t event;
     bno.getEvent(&event);


     // put your main code here, to run repeatedly:
     delay(100);                       // Here we can set the sampling rate, right now 10 Hz
     value = analogRead(IR_PIN);       // reading our analog voltage, careful we only have 10 bit resolution so each
                                       // measurement step is only 5V ÷ 1024, so our result will be 0 - 1023

     // if value is within the range of [ previous - σ , previous + σ], ignore it (if value is relatively the same)
     // this will help with having data ocuppy your buffer that is not a significant deviation.
     if( value >= (prev_value - deviation) && value <= (prev_value + deviation) )
       return;

     voltage_value = double((value*5)) / 1023; // converting to voltage [ 0, 5] v.

     distance_value = convert_to_distance(voltage_value); // getting actual distance value(cm) (careful using this, accuracy may not be ideal)
                                                          // due to the functioning of the sensor, once you're closer than around 20 cm, it will
                                                          // start predicting higher distances again. Be careful with this, this is something you can
                                                          // solve with software, however. (if previous results are close to 20 and its going down)
                                                          // then do something.... to ignore results, perhaps.


     // if this is the first loop iteration, ignore position data (always zero)
     //if its second loop iteration set the starting position for your axis
     // if its another iteration, just continue computing the rotation data

     float proximity_value = event.proximity.x;   // replace this with whatever axis you're tracking
     not_first_loop = (not_first_loop)?compute_proximity(proximity_value, &global_proximity) : true;
     float voltage = event.proximity.x;   // replace this with whatever axis you're tracking
     not_first_loop = (not_first_loop)?compute_proximity(voltage, &global_proximity) : true;

     float distance_value = event.proximity.x;   // replace this with whatever axis you're tracking
     not_first_loop = (not_first_loop)?compute_proximity(proximity_value, &global_proximity) : true;

            if(distance_value  < 20  || distance_value > 150 ) // We will ignore values outside the range of measurement, this will happen around 2.7 -0.4 v
            return;
     // Command is sent when \n (\r) or println is called
     // AT+GATTCHAR=CharacteristicID,value
     ble.print( F("AT+GATTCHAR=") );
     ble.print( proximityCharId );
     ble.print( F(",") );
     ble.print(String(global_proximity.forward_rotations));


Serial.print("Distance: ");
Serial.print(value);
Serial.print(" (0 - 1023) steps,  ");
Serial.print(voltage_value);
Serial.print(" (v),  ");
Serial.print(distance_value);
Serial.println(" cm.");
// what?



   }





   void loop(void) {

     orientation();
     proximity();


     f ( !ble.waitForOK() ) {
       error(F("Failed to get response!"));
     }

     // Delay before next measurement update
     delay(BNO055_SAMPLERATE_DELAY_MS);
   }
