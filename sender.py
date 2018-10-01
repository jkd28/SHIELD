#!/usr/bin/python3
import serial
from serial import SerialException
from Crypto.Hash import SHA256
from colorama import init
from termcolor import cprint
from pyfiglet import figlet_format
import sys


# TODO write this method
def get_file():
    return ""


# TODO write this method
def get_bytes():
    return ""


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
    connection.port = port
    connection.baudrate = baudrate
    return connection


def main():
    print_logo()
    print("Step 1) Configure serial connection settings: ")

    # configure the serial connection settings
    try:
        serial_port = input("\tEnter Serial Port: ")
        serial_rate = input("\tEnter Serial Baudrate: ")
        connection = configure_serial(serial_port, serial_rate)
    except ValueError:
        print_critical_failure("Invalid Port/Baudrate Values")
        sys.exit()

    # Attempt to connect to the serial device
    try:
        connection.open()
    except SerialException:
        print_critical_failure("Error connecting to serial device")
        sys.exit()

    print("\n\nStep 2) Select File to send, or send arbitrary bytes")
    print("--------------------------------\nTransmission Menu:\n\t" +
          "1. Send File\n\t2. Send Bytes\n--------------------------------")

    try:
        transmission_source = int(input("\tEnter Selection: "))
        while not int_within_limits(transmission_source, 1, 2):
            transmission_source = int(input("\tInvalid Selection. Enter Selection: "))
    except ValueError:
        print_critical_failure("Invalid Entry. Be sure to enter an integer.")

    if transmission_source == 1:
        to_transmit = get_file()
    elif transmission_source == 2:
        to_transmit = get_bytes()

    connection.write(to_transmit)
    connection.close()


if __name__ == "__main__":
    main()
