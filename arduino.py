import braille
import serial
from threading import Timer
from logHandler import log

arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.1)

class BrailleDisplayDriver(braille.BrailleDisplayDriver):
    name = "arduino"
    description = "Arduino Prototype"

    @classmethod
    def check(cls):
        return arduino.isOpen()

    def __init__(self):
        self.timer = Timer(.01, self.getData)
        self.timer.start()

    def terminate(self):
        super(BrailleDisplayDriver, self).terminate()
        try:
            arduino.close()
            self.timer.cancel()
        except:
            pass

    def _get_numCells(self):
        return 32

    def display(self, cells):
        arduino.write(bytes(cells))

    def getData():
        log.write("Getting data!")
        pass
