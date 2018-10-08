#!/usr/bin/python3
from colorama import init
from Crypto.Hash import SHA256
from pyfiglet import figlet_format
from serial import SerialException
from termcolor import cprint
import serial
import sys


# TODO add digital signature
def hash_data(data):
    # Hash the data
    hash = SHA256.new()
    hash.update(data.encode("UTF-8", "replace"))
    hashed_data = hash.digest()
    return hashed_data


def print_logo():
    # strip colors if stdout is redirected
    init(strip=not sys.stdout.isatty())
    logo = figlet_format('S.H.I.E.L.D.', font='starwars')
    cprint(logo, 'yellow', 'on_grey', attrs=['bold'])


def print_critical_failure(error_string):
    printed_string = "CRITICAL FAILURE: " + error_string
    print(printed_string)
    return printed_string


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
        som_code_to_avoid_Errors = 1
        # connection.open()
    except SerialException:
        print_critical_failure("Error connecting to serial device")
        sys.exit(1)

    # Send some data
    print("Step 2) Send Data:")
    user_data = input("\tEnter data to send: ")
    encrypted_transmission = hash_data(user_data)
    print(str(encrypted_transmission))
    # connection.write(encrypted_transmission)

    # connection.close()
    sys.exit(0)


if __name__ == "__main__":
    main()
