import braille
import serial
from logHandler import log

arduino = serial.Serial(port='COM3', baudrate=19200, timeout=.1)

class BrailleDisplayDriver(braille.BrailleDisplayDriver):
    name = "arduino"
    description = "Arduino Prototype"

    @classmethod
    def check(cls):
        return arduino.isOpen()

    def __init__(self):
        pass

    def terminate(self):
        super(BrailleDisplayDriver, self).terminate()
        try:
            arduino.close()
        except:
            pass

    def _get_numCells(self):
        return 32

    def display(self, cells):
        arduino.write(bytes(cells))