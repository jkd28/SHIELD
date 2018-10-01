#!/usr/bin/python3
import serial


def configure_serial(port, baudrate):
    connection = serial.Serial()
    connection.port = port
    connection.baudrate = baudrate
    return connection


def main():
    # Gather port configurations from command line
    serial_port = input("Serial Port: ")
    serial_rate = input("Serial Baudrate: ")

    serial_connection = configure_serial(serial_port, serial_rate)

    serial_connection.open()
    serial_connection.write(b'Hello')
    serial_connection.close()


if __name__ == "__main__":
    main()
