#!/usr/bin/env python3

# Import required library
import pygatt  # To access BLE GATT support
import signal  # To catch the Ctrl+C and end the program properly
import os  # To access environment variables
from dotenv import load_dotenv  # To load the environment variables from the .env file
import serial                   # To connect via the serial port
import time
import timeit

# DCD Hub
from dcd.entities.thing import Thing
from dcd.entities.property import PropertyType

# The thing ID and access token
load_dotenv()
THING_ID = os.environ['THING_ID']
THING_TOKEN = os.environ['THING_TOKEN']
BLUETOOTH_DEVICE_MAC = os.environ['BLUETOOTH_DEVICE_MAC']

# UUID of the GATT characteristic to subscribe
# GATT_CHARACTERISTIC_PROXIMITY= "02118833-4455-6677-8899-AABBCCDDEEFF"
GATT_CHARACTERISTIC_ROTATION= "06118733-4455-6677-8899-AABBCCDDEEFF"


# Many devices, e.g. Fitbit, use random addressing, this is required to connect.
ADDRESS_TYPE = pygatt.BLEAddressType.random
# Recommended number of rotation
# Did we already nudged

RECOMMENDED_NUM_ROTATION = 3
nudged = False
tired = False
proximity_value = None
total_rotation_value = None
rotation_value = 0
reseted_value = 0
dif_prev_rotation = 0




# Start reading the serial port
ser = serial.Serial(
    port = os.environ['SERIAL'],
    baudrate = 9600,
    timeout = 2)


def find_or_create(property_name, property_type):
    """Search a property by name, create it if not found, then return it."""
    if my_thing.find_property_by_name(property_name) is None:
        my_thing.create_property(name=property_name,
                                 property_type=property_type)
    return my_thing.find_property_by_name(property_name)

# Read the next line from the serial port
# and update the property values
def serial_proximity_values():
    # Read one line
    line_bytes = ser.readline()

    if len(line_bytes) > 0:
        # Convert the bytes into string
        print("Proxmity:")
        proximity_value_str = line_bytes.decode('utf-8')
        # Split the string using commas as separator, we get a list of strings
        # proximity_value = proximity_value_str.split(',')
        # Use the first element of the list as property id
        # property_id = proximity_value.pop(0)
        # # Get the property from the thing
        # prop = my_thing.properties[property_id]
        # # If we find the property, we update the values (rest of the list)

        print("Proximity:")
        print(proximity_value_str)
        #
        # if prop is not None:
        #     prop.update_value([proximity_value])
        # # Otherwise, we show a warning
        # else:
        #     print('Warning: unknown property ' + prox_property_id)
        # # Finally, we call this method again

        if proximity_value_str[0] < 100:
            check_tiredness()

        else:
            global reseted_value
            reseted_value = 0
            ser.write('0'.encode())
            print("Somebody pushing - 0 sent")


def handle_rotation_data(handle, value_bytes):
    """
    handle -- integer, characteristic read handle the data was received on
    value_bytes -- bytearray, the data returned in the notification
    """
    # decode data received from orientation sensor into rotation value
    rot_value_str = value_bytes.decode('utf-8')
    print("Received rotation data: %s (handle %d)" % (rot_value_str, handle))

    serial_proximity_values()

    try:
        global rotation_value
        global dif_prev_rotation
        global total_rotation_value
        global reseted_value
        global nudged
        total_rotation_value = float(rot_value_str)
        dif_prev_rotation = total_rotation_value - rotation_value
        rotation_value = total_rotation_value
        reseted_value += dif_prev_rotation

        print("Total rotations:")
        print(total_rotation_value)
        print("Rotations relevant for tiredness:")
        print(reseted_value)
        find_or_create("surf-wheel-rotation",
                       PropertyType.ONE_DIMENSION).update_values([rotation_value])
        print("Rotation Success 1")

    except:
        print("Can't parse - Rotation")

def check_tiredness():
    global reseted_value
    if reseted_value > RECOMMENDED_NUM_ROTATION:
        print("Tired - True - 1 sent")
        ser.write('1'.encode())


def discover_characteristic(device):
    """List characteristics of a device"""
    for uuid in device.discover_characteristics().keys():
        try:
            print("Read UUID" + str(uuid) + "   " + str(device.char_read(uuid)))
        except:
            print("Something wrong with " + str(uuid))


def read_characteristic(device, characteristic_id):
    """Read a characteristic"""
    return device.char_read(characteristic_id)


def keyboard_interrupt_handler(signal_num, frame):
    """Make sure we close our program properly"""
    print("Exiting...".format(signal_num))
    # surf_wheel.unsubscribe(GATT_CHARACTERISTIC_PROXIMITY)
    surf_wheel.unsubscribe(GATT_CHARACTERISTIC_ROTATION)
    exit(0)


# Instantiate a thing with its credential, then read its properties from the DCD Hub
my_thing = Thing(thing_id=THING_ID, token=THING_TOKEN)
my_thing.read()

# Start a BLE adapter
bleAdapter = pygatt.GATTToolBackend()
bleAdapter.start()

# Use the BLE adapter to connect to our device
surf_wheel = bleAdapter.connect(BLUETOOTH_DEVICE_MAC, address_type=ADDRESS_TYPE)

# Subscribe to the GATT service
# surf_wheel.subscribe(GATT_CHARACTERISTIC_PROXIMITY,
#                      callback=handle_proximity_data)

surf_wheel.subscribe(GATT_CHARACTERISTIC_ROTATION,
                     callback=handle_rotation_data)


# Register our Keyboard handler to exit
signal.signal(signal.SIGINT, keyboard_interrupt_handler)
