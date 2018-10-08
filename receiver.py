#!/usr/bin/python3
from colorama import init
from Crypto.Hash import SHA256
from pyfiglet import figlet_format
from serial import SerialException
from termcolor import cprint
import serial
import sys


# TODO implement digital signature
def decrypt_data(data):
    return data


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
        return [True, connection]
    except ValueError:
        return [False, connection]


def print_logo():
    # strip colors if stdout is redirected
    init(strip=not sys.stdout.isatty())
    logo = figlet_format('S.H.I.E.L.D.', font='starwars')
    cprint(logo, 'yellow', 'on_grey', attrs=['bold'])


def main():
    print_logo()
    # Configure the serial port to read from
    print("Step 1) Configure serial connection settings: ")

    serial_port = input("\tEnter Serial Port: ")
    serial_rate = input("\tEnter Serial Baudrate: ")
    [config_success, connection] = configure_serial(serial_port, serial_rate)
    if not config_success:
        print_critical_failure("Invalid Port/Baudrate Values")
        sys.exit(1)

    # Read in the data
    # while True:
    #     bytes_to_read = connection.in_waiting()
    #     data += connection.read(bytes_to_read)
    data = input("Enter TEST data: ")
    # data = data.encode("UTF-8", "replace")
    all_data = decrypt_data(data)

    # the first 32 bytes will be the hash of the data
    their_hash = all_data[0:31]
    their_test_hash = b'\x9f\x86\xd0\x81\x88L}e\x9a/\xea\xa0\xc5Z\xd0\x15\xa3\xbfO\x1b+\x0b\x82,\xd1]l\x15\xb0\xf0\n\x08'  # for the string "test"
    our_hash = hash_data(data)
    print("Our Hash: " + str(our_hash))
    print("Their Hash: " + str(their_test_hash))

    if their_test_hash == our_hash:
        print("Hash Validation Successful. Writing data to file...")
        with open("Output.txt", "w") as file:
            file.write(data)
    else:
        print("Hash Validation failed.  Hashes do not match. Exiting...")
        sys.exit(1)

    sys.exit(0)


if __name__ == "__main__":
    main()
