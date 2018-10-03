#!/usr/bin/python3
from colorama import init
from Crypto.Hash import SHA256
from pyfiglet import figlet_format
from serial import SerialException
from termcolor import cprint
import serial
import sys


# TODO write the method to retrieve a file and return
#      the bytes
def get_file():
    return ""


# TODO write the encryption method
def encrypt_and_sign(data):
    # Hash the data
    hash = SHA256.new()
    hash.update(data.encode("UTF-8", "replace"))
    hashed_data = hash.digest()
    return hashed_data


def int_within_limits(integer, min, max):
    if integer <= max and integer >= min:
        return True
    else:
        return False


def print_logo():
    # strip colors if stdout is redirected
    init(strip=not sys.stdout.isatty())
    logo = figlet_format('S.H.I.E.L.D.', font='starwars')
    cprint(logo, 'yellow', 'on_grey', attrs=['bold'])


def print_critical_failure(error_string):
    print("CRITICAL FAILURE: " + error_string)


def configure_serial(port, baudrate):
    connection = serial.Serial()
    # configure the serial connection settings
    try:
        connection.port = port
        connection.baudrate = baudrate
        return [True, connection]
    except ValueError:
        return [False, connection]


def main():
    print_logo()
    print("Step 1) Configure serial connection settings: ")

    serial_port = input("\tEnter Serial Port: ")
    serial_rate = input("\tEnter Serial Baudrate: ")
    [config_success, connection] = configure_serial(serial_port, serial_rate)
    if not config_success:
        print_critical_failure("Invalid Port/Baudrate Values")
        sys.exit(1)

    # Attempt to connect to the serial device
    try:
        #connection.open()
        some = 3
    except SerialException:
        print_critical_failure("Error connecting to serial device")
        sys.exit(1)

    # Select transmission source (bytes or file)
    print("\n\nStep 2) Select File to send, or send arbitrary bytes" +
          "\n--------------------------------" +
          "\nTransmission Menu:" +
          "\n\t1. Send File" +
          "\n\t2. Send Bytes" +
          "\n--------------------------------")

    try:
        transmission_source = int(input("\tEnter Selection: "))
        while not int_within_limits(transmission_source, 1, 2):
            transmission_source = int(input("\tInvalid Selection. Enter Selection: "))
    except ValueError:
        print_critical_failure("Invalid Entry. Be sure to enter an integer.")
        sys.exit(1)

    if transmission_source == 1:
        user_data = get_file()
    elif transmission_source == 2:
        user_data = input("Enter data to send: ")

    # Transmit selected data
    encrypted_transmission = encrypt_and_sign(user_data)
    print(encrypted_transmission)
    connection.write(encrypted_transmission)
    connection.close()


if __name__ == "__main__":
    main()
