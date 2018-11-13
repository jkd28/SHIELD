#!/usr/bin/python3
from colorama import init
from Crypto.Hash import SHA256
from pyfiglet import figlet_format
from serial import SerialException
from termcolor import cprint
import serial
import sys


def hash_data(data):
    # Hash the data
    hash = SHA256.new()
    hash.update(data.encode("UTF-8", "replace"))
    hashed_data = hash.digest()
    return hashed_data


def configure_serial(port, baudrate):
    connection = serial.Serial()
    # configure the serial connection settings
    try:
        connection.port = port
        connection.baudrate = baudrate
        connection.timeout = 30
        return [True, connection]
    except ValueError:
        return [False, connection]


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


def main():
    print_logo()
    print("Configure serial connection settings: ")

    # Configure the serial port using inputs
    serial_port = sys.argv[1]
    serial_rate = sys.argv[2]
    [config_success, connection] = configure_serial(serial_port, serial_rate)
    if not config_success:
        print_critical_failure("Invalid Port/Baudrate Values")
        sys.exit(1)

    # Attempt to connect to the serial device
    try:
        connection.open()
        print_success("Serial configured on " + connection.port +
                      " with baudrate " + str(connection.baudrate) + "bps")
    except SerialException:
        print_critical_failure("Error connecting to serial device")
        sys.exit(1)

    #TODO read in filename from initializer packet
    filename = "test.txt"
    file = open(filename, "w+")

    print("Waiting for data...")
    packet_count = 0
    while 1:
        character = connection.read(1)
        if character.decode("UTF-8", errors="replace") == 'D':
            print("Received Got Some Data! Reading...")

            # Now we expect some data
            packet_data = connection.read(1024)
            print(packet_data)
            file.write(packet_data.decode("UTF-8", errors="replace"))
            packet_count = packet_count + 1
            print("Received Packet " + str(packet_count))

    sys.exit(0)


if __name__ == "__main__":
    main()
