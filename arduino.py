import braille
import serial
import wx
import inputCore
from logHandler import log

arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.05)

class BrailleDisplayDriver(braille.BrailleDisplayDriver):
    name = "arduino"
    description = "Arduino Prototype"

    @classmethod
    def check(cls):
        return arduino.isOpen()

    def __init__(self):
        self.readTimer = wx.PyTimer(self.readData)
        #self.readTimer.Start()

    def terminate(self):
        super(BrailleDisplayDriver, self).terminate()
        try:
            arduino.close()
            self.readTimer.Stop()
            self.readTimer = None
        except:
            pass

    def _get_numCells(self):
        return 32

    def display(self, cells):
        arduino.write(bytes(cells))

    def readData(self):
        data = arduino.read()
        if data:
            log.info(data)
            try:
                inputCore.manager.executeGesture(InputGesture("bk:dot1+dot2")) 
            except inputCore.NoInputGestureAction:
                pass

class InputGesture(braille.BrailleDisplayGesture):

    source = BrailleDisplayDriver.name

    def __init__(self, keys): 
        super(InputGesture, self).__init__() 

        self.id = keys