#!/usr/bin/python3
from colorama import init
from Crypto.Hash import SHA256
from pyfiglet import figlet_format
from serial import SerialException
from struct import *
from termcolor import cprint
import serial
import struct
import sys
import time

# Define globals
PACKET_SIZE = 1024

# TODO add digital signature
def hash_data(data):
    # Hash the data
    hash = SHA256.new()
    # hash.update(data.encode("UTF-8", "replace"))
    hash.update(data)
    hashed_data = hash.digest()
    return hashed_data


def getFilenameInByteArray(filename):
    filename_bytes = bytes(filename, encoding='UTF-8')
    return filename_bytes

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
        connection.timeout = 30
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

    # Attempt to connect to the serial device
    try:
        connection.open()
        print_success("Serial configured on " + connection.port +
                      " with baudrate " + str(connection.baudrate) + "bps")
    except SerialException:
        print_critical_failure("Error connecting to serial device")
        sys.exit(1)

    # Send some data
    user_data = input("Press enter to send: ")
    while user_data == "":
        # Send start bit and wait for acknowledgement
        connection.write(bytes('S'.encode("UTF-8", "ignore")))
        ack = connection.read(2)
        if not ack.decode("UTF-8") == "RS":
            print_critical_failure("No acknowledgement of start bit")
            print("Read Value: " + ack.decode("UTF-8"))
            connection.close()
            sys.exit(1)
        else:
            print_success("Start-bit Acknowledged")

        # Open file for reading
        filename = "img/FourKiloJ.txt"
        file_data = open(filename, "rb").read()
        # print(file_data)

        # Determine number of packets based on data size
        file_size = len(file_data)
        division = divmod(file_size, PACKET_SIZE)

        number_of_packets = division[0]
        extra_data = division[1]
        if not extra_data == 0:
            number_of_packets = number_of_packets + 1

        print("File Size = " + str(file_size))
        print("Number of Packets = " + str(number_of_packets))

        # Divide data into packets
        packet_data = []
        for i in range(0, number_of_packets):
            # Packets always start at a multiple of PACKET_SIZE
            start_of_data = i * PACKET_SIZE

            if  i == number_of_packets - 1:
                end_of_data = start_of_data + extra_data
            else:
                end_of_data = (i+1) * PACKET_SIZE

            print("Packet " + str(i))
            print("\tStart: " + str(start_of_data))
            print("\tEnd  : " + str(end_of_data))
            packet_data.append(file_data[start_of_data : end_of_data])

        # Create introductory packet
        data_hash = hash_data(file_data)
        filename_bytes = getFilenameInByteArray(filename)
        initializer_packet = struct.pack('<32s32si', filename_bytes, data_hash, number_of_packets)
        print(data_hash)
        print(filename_bytes)
        print(initializer_packet)
        print(len(initializer_packet))

        print("\nWriting Initialization Packet...\n\n")
        connection.write(initializer_packet)

        ## DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG BELOW
        arduino_packets = connection.read(4)
        print("\nOur number of packets:  " + str(number_of_packets.to_bytes(4, byteorder='little')))
        print("Micro's num of packets:  " + str(arduino_packets))
        arduino_filename = connection.read(32)
        print("\nOur filename bytes:     " + str(filename_bytes))
        print("Arduino's Filename bytes: " + str(arduino_filename))
        arduino_hash = connection.read(32)
        print("\nOur hash    : " + str(data_hash))
        print("Arduino's hash: " + str(arduino_hash) + "\n\n")
        ## DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG ABOVE

        # Create all packets
        all_packets = []
        for i in range(0, number_of_packets):
            if not i == number_of_packets - 1:
                number_of_bytes = 1024
            else:
                number_of_bytes = extra_data

            packet = {
                "num_bytes" : number_of_bytes,
                "data" : packet_data[i]
            }
            all_packets.append(packet)

        # Look for a packet request
        packet_number = 0
        request = connection.read(2)
        print("Beginning Packet Transfer and Transmission")
        while request.decode("UTF-8") == "PR":
            print("Packet Requested! Packet " + str(packet_number + 1) +
                  " of " + str(number_of_packets))
            # On request, send a packet
            packet_to_send = all_packets[packet_number]

            sendable = struct.pack('<i1024s',
                                   packet_to_send['num_bytes'],
                                   packet_to_send['data']
                                  )
            #print("Sending Packet " + str(packet_number))
            connection.write(sendable)

            ## DEBUG prints
            received_packet_numbytes = connection.read(packet_to_send['num_bytes'])
            print(str(received_packet_numbytes))
            print(len(received_packet_numbytes))
            ## DEBUG above

            packet_number = packet_number + 1
            request = connection.read(2)

        if not request.decode("UTF-8") == "ER":
            print_critical_failure("Program did not reach end")
            print("Read Value: " + request.decode("UTF-8"))
            connection.close()
            sys.exit(1)
        else:
            print_success("Program Reached End!")

        # Read output data
        user_data = input("Press enter to send: ")

    print("Exiting gracefully...")
    connection.close()
    sys.exit(0)


if __name__ == "__main__":
    main()
