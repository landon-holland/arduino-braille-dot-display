import braille
import serial
import wx
import keyboard
import inputCore
from logHandler import log

arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.05)

braille_to_ascii = {
    "000000": "32",
    "011101": "33",
    "000010": "34",
    "001111": "35",
    "110101": "36",
    "100101": "37",
    "111101": "38",
    "001000": "39",
    "111011": "40",
    "011111": "41",
    "100001": "42",
    "001101": "43",
    "000001": "44",
    "001001": "45",
    "000101": "46",
    "001100": "47",
    "001011": "48",
    "010000": "49",
    "011000": "50",
    "010010": "51",
    "010011": "52",
    "010001": "53",
    "011010": "54",
    "011011": "55",
    "011001": "56",
    "001010": "57",
    "100011": "58",
    "000011": "59",
    "110001": "60",
    "111111": "61",
    "001110": "62",
    "100111": "63",
    "000100": "64",
    "100000": "65",
    "110000": "66",
    "100100": "67",
    "100110": "68",
    "100010": "69",
    "110100": "70",
    "110110": "71",
    "110010": "72",
    "010100": "73",
    "010110": "74",
    "101000": "75",
    "111000": "76",
    "101100": "77",
    "101110": "78",
    "101010": "79",
    "111100": "80",
    "111110": "81",
    "111010": "82",
    "011100": "83",
    "011110": "84",
    "101001": "85",
    "111001": "86",
    "010111": "87",
    "101101": "88",
    "101111": "89",
    "101011": "90",
    "010101": "91",
    "110011": "92",
    "110111": "93",
    "000110": "94",
    "000111": "95"
}

class BrailleDisplayDriver(braille.BrailleDisplayDriver):
    name = "arduino"
    description = "Arduino Prototype"

    @classmethod
    def check(cls):
        return arduino.isOpen()

    def __init__(self):
        self.readTimer = wx.PyTimer(self.readData)
        # self.readTimer.Start()

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
        data = int.from_bytes(data, byteorder='little', signed=False)
        if data:
            log.info(data)
            data = f"{data:08b}"
            # try:
            #     keyboard.write(chr(int(braille_to_ascii[data])))
            # except:
            #     pass
            # try:
            #     inputCore.manager.executeGesture(InputGesture("bk:dot1+dot2")) 
            # except inputCore.NoInputGestureAction:
            #     pass

class InputGesture(braille.BrailleDisplayGesture):

    source = BrailleDisplayDriver.name

    def __init__(self, keys): 
        super(InputGesture, self).__init__() 

        self.id = keys