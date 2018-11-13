import unittest
import receiver

class ReceiverTest(unittest.TestCase):
    def test_ConfigureSerial_InvalidBaudRate(self):
        [status, connection] = receiver.configure_serial("COM1", "BadBaudRate")
        self.assertEqual(False, status)


    def test_ConfigureSerial_InvalidPort(self):
        # Port must be a string
        [status, connection] = receiver.configure_serial(123423, 19200)
        self.assertEqual(False, status)


    def test_ConfigureSerial_HappyPath(self):
        [status, connection] = receiver.configure_serial("COM1", 19200)
        self.assertEqual(True, status)
        self.assertEqual("COM1", connection.port)
        self.assertEqual(19200, connection.baudrate)

    def test_HashFunction(self):
        output_hash = receiver.hash_data("test")
        expected_hash = b'\x9f\x86\xd0\x81\x88L}e\x9a/\xea\xa0\xc5Z\xd0\x15\xa3\xbfO\x1b+\x0b\x82,\xd1]l\x15\xb0\xf0\n\x08'
        self.assertEqual(expected_hash, output_hash)


def main():
    unittest.main()

if __name__ == "__main__":
    main()
