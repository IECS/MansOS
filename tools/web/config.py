#
# MansOS web server - mote configuration
#

from wmp import *
import time

# TODO: populate this from mansos Makefiles?
supportedPlatforms = ["telosb", "testbed", "testbed2", "sm3", "xm1000", "z1"]

#MAX_TIME_WAIT_FOR_REPLY = 0.1 # max time to wait for a single reply
MAX_TIME_WAIT_FOR_REPLY = 1   # max time to wait for a single reply, seconds
MAX_RETRIES = 3               # send 3 times before giving up

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

def isFatCharacterAcceptable(c):
    if c.isalnum(): return True
    if c == ' ': return True
    if c >= 128: return True
    if c == '!' or c == '#' \
            or c == '$' or c == '%' \
            or c == '&' or c == '\'' \
            or c == '(' or c == ')' \
            or c == '-' or c == '@' \
            or c == '^' or c == '_' \
            or c == '`' or c == '{' \
            or c == '}' or c == '~':
        return True
    return False


def isValidFatFilename(filename):
    baseLen = 0
    extLen = 0
    numDots = 0
    for c in filename:
        if c == '.' and numDots == 0:
            numDots += 1
            continue

        if numDots == 0:
            baseLen += 1
        else:
            extLen += 1

        if not isFatCharacterAcceptable(c):
            return False

    return baseLen >= 1 and baseLen <= 8 and extLen <= 3

def fatFilenamePrettify(filename):
    result = ""
    for c in filename:
        if c != ' ' and isFatCharacterAcceptable(c):
            result += c
    return result


class Platform(object):
    def __init__(self, name, sensors, outputs, leds):
        self.name = name
        self.sensors = sensors
        self.outputs = outputs
        self.leds = leds


def wmpSendCommand(ser, command, args):
    print "send cmd=", command, "args=", args
    txFrame = "%c%c%c" % (WMP_START_CHARACTER, command, len(args))
    for a in args:
        txFrame += chr(a)

    crc = 0
    for c in txFrame:
        crc ^= ord(c)
        ser.write(bytearray([ord(c)]))
    ser.write(bytearray([crc]))

    # print "command sent!"


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
                self.platforms[platform] = Platform(platform,
                                                    module.sensors, module.outputs, module.leds)


    def wmpProcessCommand(self):
        #print "wmpProcessCommand"
        print "rcdv command=", (self.currentSp.command & ~WMP_CMD_REPLY_FLAG), " args=", self.currentSp.arguments
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

    def wmpExchangeCommand(self, command, arguments):
#        print self.mote.port
        returnArguments = [] # default
        ok = False
        for i in range(MAX_RETRIES):
            iterEndTime = time.time() + MAX_TIME_WAIT_FOR_REPLY
            self.lastSp = None
            wmpSendCommand(self.mote.port, command, arguments)
            time.sleep(0.01)
            while time.time() < iterEndTime:
                if self.lastSp:
                    if self.lastSp.command == command:
                        # print "  command OK"
                        returnArguments = self.lastSp.arguments
                        ok = True
                    else:
                        # print "  wrong/unexpected command"
                        pass
                    self.lastSp = None
                    break
                # print "waiting..."
                time.sleep(0.01)
            if ok: break
        if not ok: print "reply NOT received!"
        return returnArguments

    def wmpGetSensorConfig(self, sensorCode):
        args = self.wmpExchangeCommand(WMP_CMD_GET_SENSOR, [sensorCode])
        if len(args) < 5:
            return 0 # default
        return le32read(args[1:])

    def wmpGetOutputConfig(self, sensorCode):
        args = self.wmpExchangeCommand(WMP_CMD_GET_OUTPUT, [sensorCode])
        if len(args) < 2:
            return False # default
        return bool(args[1])

    def wmpGetLedConfig(self, ledCode):
        args = self.wmpExchangeCommand(WMP_CMD_GET_LED, [ledCode])
        if len(args) < 2:
            return False # default
        return bool(args[1])

    def updateConfigValues(self, qs):
        if "set" not in qs:
            return

        for s in self.activePlatform.sensors:
            if s.varname in qs:
                try:
                    t = int(qs[s.varname][0], 0)
                    s.period = t
                except:
                    pass

        for s in self.activePlatform.outputs:
            if s.varname in qs:
                try:
                    t = qs[s.varname][0] == "on"
                    s.isSelected = t
                except:
                    pass
            else:
                s.isSelected = False

        for s in self.activePlatform.leds:
            if s.varname in qs:
                try:
                    t = qs[s.varname][0] == "on"
                    s.isOn = t
                except:
                    pass
            else:
                s.isOn = False

    def getConfigValues(self):
        (errstr, ok) = self.checkValid()
        if not ok:
            return "<strong>Get failed: " + errstr + "</strong><br/>"

        self.configMode = True

        for s in self.activePlatform.sensors:
            s.period = self.wmpGetSensorConfig(s.code)

        for s in self.activePlatform.outputs:
            s.isSelected = self.wmpGetOutputConfig(s.code)

        for s in self.activePlatform.leds:
            s.isOn = self.wmpGetLedConfig(s.code)

        self.configMode = False
        return "<strong>Configuration values read!</strong><br/>"


    def setConfigValues(self):
        (errstr, ok) = self.checkValid()
        if not ok:
            return "<strong>Get failed: " + errstr + "</strong><br/>"

        self.configMode = True

        for s in self.activePlatform.sensors:
            args = [s.code]
            args += le32write(s.period)
            self.wmpExchangeCommand(WMP_CMD_SET_SENSOR, args)

        for s in self.activePlatform.outputs:
            args = [s.code, int(s.isSelected)]
            self.wmpExchangeCommand(WMP_CMD_SET_OUTPUT, args)

        for s in self.activePlatform.leds:
            args = [s.code, int(s.isOn)]
            self.wmpExchangeCommand(WMP_CMD_SET_LED, args)

        print "done!"
        self.configMode = False
        return "<strong>Configuration values written!</strong><br/>"


    def setMote(self, mote, platform):
        if self.mote and self.mote != mote:
            self.mote.closeSerial()
            time.sleep(0.1)

        self.mote = mote
        self.activePlatform = self.platforms.get(platform, None)

        if not self.activePlatform:
            print("platform not supported or sensorlist file not present!")
            self.mote = None
            return

        self.mote.platform = platform
        if not self.mote.port:
            self.mote.tryToOpenSerial(True)
            time.sleep(2)
        if not self.mote.port:
            # self.mote = None
            return


    def getFileContentsHTML(self, qs):
        (errst, ok) = self.checkValid()
        if not ok:
            return (errst, False)

        if "filename" not in qs:
            errst = "file name no specified!"
            return (errst, False)

        filename = qs["filename"][0]
        if not isValidFatFilename(filename):
            errst = 'file name "' + filename + '" is not valid!'
            return (errst, False)

        self.configMode = True
        args = self.wmpExchangeCommand(WMP_CMD_GET_FILE, bytearray(filename))
        self.configMode = False

        contents = str(bytearray(args))

        text = 'File ' + filename + ' contents:<br/>\n'
        text += contents
        text += '\n</br>\n'

        return (text, True)


    def getFileListHTML(self, moteIndex):
        (errst, ok) = self.checkValid()
        if not ok:
            return (errst, False)

        self.configMode = True
        args = self.wmpExchangeCommand(WMP_CMD_GET_FILELIST, [])
        self.configMode = False

        namelist = str(bytearray(args)).split('\n')

        motename = "mote" + str(moteIndex)

        if len(namelist) == 0:
            text = "The SD card is empty!"
        else:
            text = '<strong>Files:</strong><br/>\n'
            text += '<div class="files">'
            for filename in namelist:
                text += '<div class="entry">'
                filename = fatFilenamePrettify(filename)
                text += filename
                text += '&nbsp;&nbsp;<a href="config?filename=' + filename + '&' \
                    + motename + '_files=1&sel_' + motename + '=' + self.activePlatform.name \
                    + '">File contents</a>'
                text += "</div>\n"
            text += "</div>"

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
            text += '<div class="entry">'
            text += toTitleCase(s.name) + ": "
            text += '<input type="text" name="' + s.varname + '" value="' + str(s.period) + '"/><br/></div>\n'
        text += '</div>\n'

        # outputs
        text += '<strong>System outputs:</strong><br/>\n'
        text += '<div class="subcontents">\n'
        for s in self.activePlatform.outputs:
            text += '<div class="entry">'
            text += '<input type="checkbox" name="' + s.varname + '"'
            if s.isSelected: text += ' checked="checked"'
            text += '/> Write to ' + toCamelCase(s.name)
            if s.varname == "file":
                text += '<br/><label for="filename">Filename: </label>'
                text += '<input type="input" name="filename"/>\n'
            text += '<br/></div>\n'
        text += '</div>\n'

        # leds
        text += '<strong>LED:</strong><br/>\n'
        text += '<div class="subcontents">\n'
        for s in self.activePlatform.leds:
            text += '<div class="entry">'
            text += '<input type="checkbox" name="' + s.varname + '"'
            if s.isOn: text += ' checked="checked"'
            text += '/> ' + s.name + " LED on"
            text += '<br/></div>\n'
        text += '</div>\n'

        # end of form
        motename = "mote" + str(self.mote.number)
        text += '<input type="hidden" name="' + motename + '_cfg" value="1"/>\n'
        text += '<input type="hidden" name="sel_' + motename \
            + '" value="' + self.activePlatform.name + '" />\n'
        text += '<input type="submit" name="get" value="Get values"/> \n'
        text += '<input type="submit" name="set" value="Set values"/>\n'
        text += '</div></form>\n'
        return (text, True)

# global variable
configInstance = Config()
