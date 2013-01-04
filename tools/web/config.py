from constants import *
from wmp import *
import time

def toTitleCase(s):
    if s == '': return ''
    return s[0].upper() + s[1:]

def toCamelCase(s):
    if s == '': return ''
    if len(s) > 1 and s[0].isupper() and s[1].isupper():
        # acronyms are unchanged
        return s
    return s[0].lower() + s[1:]

def le32read(args):
    result = args[3] << 24
    result += args[2] << 16
    result += args[1] << 8
    result += args[0]
    return result

def le32write(number):
    args = [0, 0, 0, 0]
    args[0] = number & 0xff
    args[1] = (number >> 8) & 0xff
    args[2] = (number >> 16) & 0xff
    args[3] = (number >> 24) & 0xff
    return args


# TODO: populate this from mansos Makefiles?
supportedPlatforms = ["telosb", "testbed", "testbed2", "sm3"]

class Platform(object):
    def __init__(self, name, sensors, outputs):
        self.name = name
        self.sensors = sensors
        self.outputs = outputs


def wmpSendCommand(ser, command, args):
    txFrame = "%c%c%c" % (WMP_START_CHARACTER, command, len(args))
    for a in args:
        txFrame += chr(a)

    crc = 0
    for c in txFrame:
        crc ^= ord(c)
        ser.write(bytearray([ord(c)]))
    ser.write(bytearray([crc]))

    print "command sent!"


class SerialPacket(object):
    def __init__(self):
        self.command    = 0  # WMP command
        self.argLen     = 0  # promised argument length
        self.arguments  = [] # variable length arguments
        self.crc        = 0  # XOR of packet fields

    def wmpCrc(self):
        result = ord(WMP_START_CHARACTER)
        result ^= self.command
        result ^= self.argLen
        for a in self.arguments:
            result ^= a
        return result


#
# Class that manages configuration getting and setting
#
class Config(object):
    READ_START_CHARACTER = 0
    READ_COMMAND = 1
    READ_ARG_LEN = 2
    READ_ARGS = 3
    READ_CRC = 4

    def __init__(self):
        self.configMode = False
        self.mote = None
        self.activePlatform = None
        self.platforms = {}
        self.state = self.READ_START_CHARACTER
        self.currentSp = SerialPacket()
        self.lastSp = None
        for platform in supportedPlatforms:
            try:
                module = __import__("sensorlist_" + platform)
            except:
                module = None
            if module:
                self.platforms[platform] = Platform(platform, module.sensors, module.outputs)


    def wmpProcessCommand(self):
        print "wmpProcessCommand"
        print "command=", self.currentSp.command
        if not (self.currentSp.command & WMP_CMD_REPLY_FLAG):
            print "not a reply, ignoring"
            return

        if self.lastSp:
            print "previous reply not processed, ignoring"
            return

        self.lastSp = self.currentSp
        self.lastSp.command &= ~WMP_CMD_REPLY_FLAG


    def byteRead(self, x):
        assert self.configMode
#        print "read", ord(x)

        if self.state == self.READ_START_CHARACTER:
            if x == WMP_START_CHARACTER:
                self.state = self.READ_COMMAND

        elif self.state == self.READ_COMMAND:
            self.currentSp.command = ord(x)
            self.state = self.READ_ARG_LEN

        elif self.state == self.READ_ARG_LEN:
            self.currentSp.argLen = ord(x)
            if self.currentSp.argLen:
                self.state = self.READ_ARGS
            else:
                self.state = self.READ_CRC

        elif self.state == self.READ_ARGS:
            self.currentSp.arguments.append(ord(x))
            if len(self.currentSp.arguments) == self.currentSp.argLen:
                self.state = self.READ_CRC

        elif self.state == self.READ_CRC:
            self.currentSp.crc = ord(x)
            if self.currentSp.wmpCrc() == self.currentSp.crc:
                self.wmpProcessCommand()
            else:
                print("bad crc {:x}\n".format(sp.crc))
            self.currentSp = SerialPacket()
            self.state = self.READ_START_CHARACTER


    def checkValid(self):
        if not self.mote:
            return ("mote not present!", False)
        if not self.mote.port:
            return ("serial port " + self.mote.portName + " not opened!", False)
        if not self.activePlatform:
            return ("platform not selected!", False)
        return (None, True)


    def getConfigValues(self):
        (errstr, ok) = self.checkValid()
        if not ok:
            return "<strong>Get failed: " + errstr + "</strong><br/>"

        self.configMode = True

        for s in self.activePlatform.sensors:
            print "process sensor", s.varname
            wmpSendCommand(self.mote.port, WMP_CMD_GET_SENSOR, [s.code])
            while True:
                if self.lastSp and self.lastSp.command == WMP_CMD_GET_SENSOR:
                    s.period = le32read(self.lastSp.arguments[1:])
                    self.lastSp = None
                    break
#                print "waiting..."
                time.sleep(0.1)

        for s in self.activePlatform.outputs:
            print "process output", s.varname
            wmpSendCommand(self.mote.port, WMP_CMD_GET_OUTPUT, [s.code])
            while True:
                if self.lastSp and self.lastSp.command == WMP_CMD_GET_OUTPUT:
                    s.selected = bool(self.lastSp.arguments[1])
                    self.lastSp = None
                    break
#                print "waiting..."
                time.sleep(0.1)

        self.configMode = False
        return "<strong>Configuration values read!</strong><br/>"


    def setConfigValues(self):
        (errstr, ok) = self.checkValid()
        if not ok:
            return "<strong>Get failed: " + errstr + "</strong><br/>"

        self.configMode = True

        for s in self.activePlatform.sensors:
            print "process sensor", s.varname
            args = [s.code]
            args.extend(le32write(s.period))
            wmpSendCommand(self.mote.port, WMP_CMD_SET_SENSOR, args)
            while True:
                if self.lastSp and self.lastSp.command == WMP_CMD_SET_SENSOR:
                    self.lastSp = None
                    break
#                print "waiting..."
                time.sleep(0.1)

        for s in self.activePlatform.outputs:
            print "process output", s.varname
            args = [s.code, int(s.selected)]
            wmpSendCommand(self.mote.port, WMP_CMD_SET_OUTPUT, args)
            while True:
                if self.lastSp and self.lastSp.command == WMP_CMD_SET_OUTPUT:
                    self.lastSp = None
                    break
#                print "waiting..."
                time.sleep(0.1)

        print "done!"
        self.configMode = False
        return "<strong>Configuration values written!</strong><br/>"


    def setMote(self, mote, platform):
        if self.mote:
            self.mote.closeSerial()

        self.mote = mote
        self.activePlatform = self.platforms.get(platform, None)

        if not self.activePlatform:
            print("platform not supported or sensorlist file not present!")
            self.mote = None
            return

        self.mote.platform = platform
        self.mote.tryToOpenSerial()
        if not self.mote.port:
            # self.mote = None
            return


    def getFileListHTML(self):
        (errst, ok) = self.checkValid()
        if not ok:
            return (errst, False)

        text = 'File list will be here!\n'
        return (text, True)


    def getConfigHTML(self):
        (errst, ok) = self.checkValid()
        if not ok:
            return (errst, False)

        # start of form
        text = '<form action="config"><div class="form">\n'

        # mote
        text += '<strong>Mote:</strong><br/>\n'
        text += '<div class="subcontents">\n'
        text += 'Port: <em>' + self.mote.portName + '</em><br/>\n'
        text += 'Platform: <em>' + self.activePlatform.name + '</em><br/>\n'
        text += '</div>\n'

        # sensors
        text += '<strong>Sensor reading periods (in milliseconds, "0" means disabled):</strong><br/>\n'
        text += '<div class="subcontents">\n'
        for s in self.activePlatform.sensors:
            text += '<div class="sensor">'
            text += toTitleCase(s.name) + ": "
            text += '<input type="text" name="' + s.varname + '" value="' + str(s.period) + '"/><br/></div>\n'
        text += '</div>\n'

        # outputs
        text += '<strong>System outputs:</strong><br/>\n'
        text += '<div class="subcontents">\n'
        for s in self.activePlatform.outputs:
            text += '<div class="output">'
            text += '<input type="checkbox" name="' + s.varname + '"'
            if s.selected: text += ' checked="checked"'
            text += '/> Write to ' + toCamelCase(s.name)
            if s.varname == "file":
                text += '<br/><label for="filename">Filename: </label>'
                text += '<input type="input" name="filename"/>\n'
            text += '<br/></div>\n'
        text += '</div>\n'

        # end of form
        motename = "mote" + str(self.mote.number)
        text += '<input type="hidden" name="' + motename + '_cfg" value="1"/>\n'
        text += '<input type="hidden" name="sel_' + motename + '" value="' + self.activePlatform.name + '" />\n'
        text += '<br/><input type="submit" name="get" value="Get values"/> '
        text += '<input type="submit" name="set" value="Set values"/>\n'
        text += '</div></form>\n'
        return (text, True)

# global variable
configInstance = Config()
