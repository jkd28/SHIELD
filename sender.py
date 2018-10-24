#!/usr/bin/python3
from colorama import init
from Crypto.Hash import SHA256
from pyfiglet import figlet_format
from serial import SerialException
from struct import *
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


def print_success(success_string):
    printed_string = "SUCCESS! " + success_string
    print(printed_string)
    return printed_string


def print_usage():
    usage_string = "\nUSAGE: python sender.py [port] [baudrate]\n"
    print(usage_string)
    return usage_string


def configure_serial(port, baudrate):
    connection = serial.Serial()
    # configure the serial connection settings
    try:
        connection.port = port
        connection.baudrate = baudrate
        connection.timeout = 10
        return [True, connection]
    except ValueError:
        return [False, connection]


def main():
    if not len(sys.argv) == 3:
        print_critical_failure("Invalid usage")
        print_usage()
        sys.exit(1)

    print_logo()

    # Configure the serial port using inputs
    serial_port = sys.argv[1]
    serial_rate = sys.argv[2]
    [config_success, connection] = configure_serial(serial_port, serial_rate)
    if not config_success:
        print_critical_failure("Invalid Port/Baudrate Values")
        sys.exit(1)
    else:
        print_success("Serial configured on " + serial_port +
                      " with baudrate " + serial_rate + "bps")

    # Attempt to connect to the serial device
    try:
        som_code_to_avoid_Errors = 1
        connection.open()
    except SerialException:
        print_critical_failure("Error connecting to serial device")
        sys.exit(1)

    # Send some data
    user_data = input("Enter data to send: ")
    while not user_data == "":
        connection.write(bytes('S'.encode("UTF-8", "ignore")))
        ack = connection.read(2)
        if not ack.decode("UTF-8") == "RS":
            print_critical_failure("No acknowledgement of start bit")
            connection.close()
            sys.exit(1)
        else:
            print_success("Start-bit Acknowledged")

        connection.write(bytes("hello world".encode("UTF-8", "replace")))

        connection.write(bytes('E'.encode("UTF-8", "ignore")))
        ack = connection.read_until()
        if not ack.decode("UTF-8") == "RE":
            print_critical_failure("No acknowledgement of start bit")
            connection.close()
            sys.exit(1)
        else:
            print_success("End-bit Acknowledged")



        # Read output data
        user_data = input("Enter data to send: ")

    print("Exiting gracefully...")
    connection.close()
    sys.exit(0)


if __name__ == "__main__":
    main()
