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
GATT_CHARACTERISTIC_PROXIMITY= "02118833-4455-6677-8899-AABBCCDDEEFF"
GATT_CHARACTERISTIC_ROTATION= "06118733-4455-6677-8899-AABBCCDDEEFF"


# Many devices, e.g. Fitbit, use random addressing, this is required to connect.
ADDRESS_TYPE = pygatt.BLEAddressType.random
# Recommended number of rotation
# Did we already nudged

RECOMMENDED_NUM_ROTATION = 3
nudged = False
nobodybehind = False
tired = False

proximity_value = None
rotation_value = 0

rotation_being_pushed = 0
dif_prev_rotation = 0

# Start reading the serial port
ser = serial.Serial(
    port = os.environ['SERIAL'],
    baudrate = 9600,
    timeout = 2)

# USER_STATUS_BEHIND = 600

def find_or_create(property_name, property_type):
    """Search a property by name, create it if not found, then return it."""
    if my_thing.find_property_by_name(property_name) is None:
        my_thing.create_property(name=property_name,
                                 property_type=property_type)
    return my_thing.find_property_by_name(property_name)


def handle_proximity_data(handle, value_bytes):
    """
    handle -- integer, characteristic read handle the data was received on
    value_bytes -- bytearray, the data returned in the notification
    """

    print(str(value_bytes))
    value_str = value_bytes.decode('utf-8')
    print(value_str)

    try:
        print("Received data: %s (handle %d)" % (value_str, handle))
        global proximity_value
        proximity_value = float(value_str)
        find_or_create("Surf Wheel Proximity",
                       PropertyType.PROXIMITY).update_values([proximity_value])
        check_tiredness()

    except:
        print("cant parse " + str(value_bytes))

def handle_rotation_data(handle, value_bytes):
    """
    handle -- integer, characteristic read handle the data was received on
    value_bytes -- bytearray, the data returned in the notification
    """
    value_str = value_bytes.decode('utf-8')
    print("Received data: %s (handle %d)" % (value_str, handle))

    try:
        total_rotation_value = float(value_str)

        global rotation_value, dif_prev_rotation
        dif_prev_rotation = total_rotation_value - rotation_value
        rotation_value = total_rotation_value
        print(rotation_value)

        find_or_create("surf-wheel-rotation",
                       PropertyType.ONE_DIMENSION).update_values([rotation_value])

        check_tiredness()
    #
    # else:
    #     ser.write('0'.encode())
    #     print("User is not tired - 0 sent")

    except:
        print("cant parse")

def check_tiredness():
    if proximity_value is None or rotation_value is None:
        return

    # when someone is pushing the number of max rotations should be reset
    # so that when they leave the threshold is higher
    if proximity_value < 440:
        nobodybehind = True

    else:
        nobodybehind = False
        rotation_being_pushed += dif_prev_rotation

    # above recommendation and self propelled
    if rotation_value-rotation_being_pushed > RECOMMENDED_NUM_ROTATION:
        tired = True

    # if tired and not nudged:

    if tired:
        ser.write('1'.encode())
        print("User is tired - 1 sent")
        global nudged
        nudged = True

    else:
        ser.write('0'.encode())
        print("User is not tired - 0 sent")


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
    surf_wheel.unsubscribe(GATT_CHARACTERISTIC_PROXIMITY)
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
surf_wheel.subscribe(GATT_CHARACTERISTIC_PROXIMITY,
                     callback=handle_proximity_data)

surf_wheel.subscribe(GATT_CHARACTERISTIC_ROTATION,
                     callback=handle_rotation_data)


# Register our Keyboard handler to exit
signal.signal(signal.SIGINT, keyboard_interrupt_handler)
