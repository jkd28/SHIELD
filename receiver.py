#!/usr/bin/python3
from colorama import init
from Crypto.Hash import SHA256
from pyfiglet import figlet_format
from serial import SerialException
from termcolor import cprint
import locale
import serial
import struct
import sys


def hash_data(data):
    # Hash the data
    hash = SHA256.new()
    #hash.update(data.encode("UTF-8", "replace"))
    hash.update(data)
    hashed_data = hash.digest()
    return hashed_data


def configure_serial(port):
    connection = serial.Serial()
    # configure the serial connection settings
    try:
        connection.port = port
        connection.baudrate = 115200
        connection.timeout = 20
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
    [config_success, connection] = configure_serial(serial_port)
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

    print("Getting Information Packet: ")
    info_packet_struct = connection.read(72)
    print("Info Packet Received")
    print(info_packet_struct)

    information = struct.unpack('<32s32sii', info_packet_struct)
    # parse filename
    received_filename = information[0].decode(locale.getdefaultlocale()[1]).rstrip('\0')
    # extension = filename.split('.')[1]
    # filename_beginning = filename.split('.')[0]
    # filename = filename_beginning + "test." + extension
    filename = "test/RECEIVED" + received_filename


    # parse other information
    data_hash = information[1]
    num_packets = information[2]
    num_bytes_in_last_packet = information[3]

    print("Filename: " + filename)
    print("Hash: " + str(data_hash))
    print("Num Packets: " + str(num_packets))
    print("Last Packet Bytes: " + str(num_bytes_in_last_packet))

    print("\n\rWaiting for data")
    packets = []
    packet_count = 0
    while packet_count < num_packets:
        if packet_count == (num_packets - 1):
            # Last packet does not have a kilobyte
            expected_bytes = num_bytes_in_last_packet
        else:
            expected_bytes = 1024

        # Now we expect some data
        packet_data = connection.read(expected_bytes)
        packets.append(packet_data)
        packet_count = packet_count + 1

        print("Packet " + str(packet_count) + " of " + str(num_packets) + " Received: " + str(len(packet_data)) + " bytes")
        print(packet_data)
        print()

    # Once finished reading each packet,
    our_hash = ''
    file = open(filename, "wb")
    for packet in packets:
        file.write(packet)

    file.close()

    written_file = open(filename, "rb").read()
    our_hash = hash_data(written_file)
    print("Computed Hash: " + str(our_hash))

    print("Comparing Hashes....")
    if(data_hash == our_hash):
        print("SUCCESS! HASHES MATCH!!")
    else:
        print("Hashes do not match.")

    sys.exit(0)


if __name__ == "__main__":
    main()
