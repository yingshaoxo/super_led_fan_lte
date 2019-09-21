"""
author: yingshaoxo
gmail: yingshaoxo@gmail.com

ls -l /dev/ttyUSB0
sudo usermod -a -G uucp yingshaoxo
sudo chmod a+rw /dev/ttyUSB0
"""
import serial
import binascii
from time import sleep


def int_to_byte(integer):
    hex_string = '{:02x}'.format(integer)
    a_byte = binascii.unhexlify(hex_string)
    return a_byte


def byte_to_int(a_byte):
    hex_string = binascii.hexlify(a_byte)
    integer = int(hex_string, 16)
    return integer


def byte_to_string(a_byte):
    return format(byte_to_int(a_byte), "X")


def hex_to_bytes(a_hex_string):
    return bytes.fromhex(a_hex_string)


class SmartOpen_LCD():
    def __init__(self):
        self.serial = serial.Serial('/dev/ttyUSB0', 9600)  # open serial port
        print()
        print('-'*20)
        print(self.serial.name)         # check which port was really used
        print('-'*20)
        print()

        self.color_table = {
            "black": "0000",
            "blue": "001f",
            "red": "f800",
            "green": "07e0",
            "cyan": "07ff",
            "magenta": "f81f",
            "yellow": "ffe0",
            "white": "ffff",
        }

        self.wait(1.5)

    def wait(self, time=1):
        sleep(time)

    def write_command(self, hex_string):
        if self.serial.writable():
            self.serial.write(hex_to_bytes(hex_string))
            print(hex_string)
            self.wait(0.1)

    def reset(self):
        self.write_command("7E0205EF")

    def set_blacklight(self, brightness):
        brightness_hex = byte_to_string(int_to_byte(brightness))
        self.write_command(f"7E0306{brightness_hex}EF")

    def fill_screen(self, color="white"):
        if color in self.color_table.keys():
            color = self.color_table[color]
        self.write_command(f"7E0420{color}EF")


color_table = {
    "black": "0000",
    "blue": "001f",
    "red": "f800",
    "green": "07e0",
    "cyan": "07ff",
    "magenta": "f81f",
    "yellow": "ffe0",
    "white": "ffff",
}

lcd = SmartOpen_LCD()

# lcd.reset()
lcd.set_blacklight(150)

while 1:
    for color in color_table.keys():
        lcd.fill_screen(color)
        lcd.wait(1)
