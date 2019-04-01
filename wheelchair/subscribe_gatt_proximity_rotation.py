#!/usr/bin/env python3

# Import required library
import pygatt  # To access BLE GATT support
import signal  # To catch the Ctrl+C and end the program properly
import os  # To access environment variables
from dotenv import load_dotenv  # To load the environment variables from the .env file

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
# RECOMMENDED_NUM_ROTATION = 1
# Did we already nudged
nudged = False

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
        proximity_values = [float(value_str)]
        find_or_create("Surf Wheel Proximity",
                       PropertyType.PROXIMITY).update_values(proximity_values)
        if proximity_values > 440:
            ser.write('1')
            time.sleep(8)
            ser.write('0')
            global nudged
            nudged = True
    except:
        print("cant parse " + str(value_bytes))

def handle_rotation_data(handle, value_bytes):
    """
    handle -- integer, characteristic read handle the data was received on
    value_bytes -- bytearray, the data returned in the notification
    """
    RECOMMENDED_NUM_ROTATION = 1
    value_str = value_bytes.decode('utf-8')
    print("Received data: %s (handle %d)" % (value_str, handle))

    try:
        rotation_values = float(value_str)
        print(rotation_values)

        find_or_create("surf-wheel-rotation-df50",
                       PropertyType.ONE_DIMENSION).update_values([rotation_values])

        print("findorcreate")

        # while rotation_values > RECOMMENDED_NUM_ROTATION:
        #     ser.write('1')
        #     time.sleep(3)
        #     RECOMMENDED_NUM_ROTATION = RECOMMENDED_NUM_ROTATION + rotation_vlues

        if rotation_values > 1:
            print("i am inside")
            ser.write('1'.encode())
            print("i am inside 2")
            time.sleep(3)
            print("in am inside 3")
            ser.write('0'.encode())
            print("before global")
            global nudged
            print("after global")
            nudged = True
    except:
        print("cant parse")

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
