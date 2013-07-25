#
# MansOS web server - mote configuration
#

from __future__ import print_function
from wmp import *
import os
import configuration
import time
import utils

# names of all platforms the web interface supports.
# TODO: populate this from Makefiles?
supportedPlatforms = ["telosb", "testbed", "testbed2", "sm3", "xm1000"]

MAX_TIME_WAIT_FOR_REPLY = 0.1 # max time to wait for a single reply, seconds
MAX_RETRIES = 3               # send 3 times before giving up


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
    print("send cmd=" + str(command) + " args=" + str(map(hex, args)))
    txFrame = "%c%c%c" % (WMP_START_CHARACTER, command, len(args))
    for a in args:
        txFrame += chr(a)

    crc = 0
    for c in txFrame:
        crc ^= ord(c)
        ser.write(bytearray([ord(c)]))
    ser.write(bytearray([crc]))


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
class MoteConfig(object):
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
        self.filenameOnMote = configuration.c.getCfgValue("saveToFilenameOnMote")
        for platform in supportedPlatforms:
            try:
                module = __import__("sensorlist_" + platform)
            except:
                module = None
            if module:
                self.platforms[platform] = Platform(platform,
                                                    module.sensors, module.outputs, module.leds)


    def wmpProcessCommand(self):
        print("rcdv command=" + str(self.currentSp.command & ~WMP_CMD_REPLY_FLAG) +\
            " args=" + str(map(hex, self.currentSp.arguments)))
        if not (self.currentSp.command & WMP_CMD_REPLY_FLAG):
            print("not a reply, ignoring")
            return

        if self.lastSp:
            print("previous reply not processed, ignoring")
            return

        self.lastSp = self.currentSp
        self.lastSp.command &= ~WMP_CMD_REPLY_FLAG


    def byteRead(self, x):
        assert self.configMode

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
            return ("serial port " + self.mote.getPortName() + " not opened!", False)
        if not self.activePlatform:
            return ("platform not selected!", False)
        return (None, True)

    def wmpExchangeCommand(self, command, arguments):
        returnArguments = [] # default
        ok = False
        try:
            for i in range(MAX_RETRIES):
                iterEndTime = time.time() + MAX_TIME_WAIT_FOR_REPLY
                self.lastSp = None
                wmpSendCommand(self.mote.port, command, arguments)
                time.sleep(0.01)
                while time.time() < iterEndTime:
                    if self.lastSp:
                        if self.lastSp.command == command:
                            # print("  command OK")
                            returnArguments = self.lastSp.arguments
                            ok = True
                        else:
                            # print("  wrong/unexpected command")
                            pass
                        self.lastSp = None
                        break
                    # print("waiting...")
                    time.sleep(0.01)
                if ok: break
        except Exception as e:
            pass
        if not ok: print("reply NOT received!")
        return (returnArguments, ok)

    def wmpGetSensorConfig(self, sensorCode):
        (args, ok) = self.wmpExchangeCommand(WMP_CMD_GET_SENSOR, [sensorCode])
        if not ok or len(args) < 5:
            return 0 # default
        return utils.le32read(args[1:])

    def wmpGetOutputConfig(self, sensorCode):
        (args, ok) = self.wmpExchangeCommand(WMP_CMD_GET_OUTPUT, [sensorCode])
        if not ok or len(args) < 2:
            return False # default
        return bool(args[1])

    def wmpGetLedConfig(self, ledCode):
        (args, ok) = self.wmpExchangeCommand(WMP_CMD_GET_LED, [ledCode])
        if not ok or len(args) < 2:
            return False # default
        return bool(args[1])

    def updateConfigValues(self, qs):
        if "set" not in qs:
            return (None, True)

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

        if "filename" in qs:
            newFilename = qs["filename"][0]
            if len(newFilename) and newFilename.find(".") == -1:
                # add default extension
                newFilename += ".csv"
            if len(newFilename) == 0 or isValidFatFilename(newFilename):
                self.filenameOnMote = newFilename
            else:
                return ("The filename specified is not a valid FAT file name!", False)
        else:
            self.filenameOnMote = ""

        configuration.c.setCfgValue("saveToFilenameOnMote", self.filenameOnMote)
        configuration.c.save()
        return (None, True)


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

        (args, ok) = self.wmpExchangeCommand(WMP_CMD_GET_FILENAME, [])
        newFilename = "".join(args)
        if ok and newFilename != self.filenameOnMote:
            self.filenameOnMote = newFilename
            configuration.c.setCfgValue("saveToFilenameOnMote", self.filenameOnMote)
            configuration.c.save()

        self.configMode = False
        return "<strong>Configuration values read!</strong><br/>"


    def setConfigValues(self):
        (errstr, ok) = self.checkValid()
        if not ok:
            return "<strong>Get failed: " + errstr + "</strong><br/>"

        self.configMode = True

        for s in self.activePlatform.sensors:
            args = [s.code]
            args += utils.le32write(s.period)
            self.wmpExchangeCommand(WMP_CMD_SET_SENSOR, args)

        for s in self.activePlatform.outputs:
            args = [s.code, int(s.isSelected)]
            self.wmpExchangeCommand(WMP_CMD_SET_OUTPUT, args)

        for s in self.activePlatform.leds:
            args = [s.code, int(s.isOn)]
            self.wmpExchangeCommand(WMP_CMD_SET_LED, args)

        args = bytearray(self.filenameOnMote)
        self.wmpExchangeCommand(WMP_CMD_SET_FILENAME, args)

        self.configMode = False
        return "<strong>Configuration values written!</strong><br/>"


    def setMote(self, mote, platform):
        if self.mote and self.mote != mote:
            self.mote.ensureSerialIsClosed()
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
        (args, ok) = self.wmpExchangeCommand(WMP_CMD_GET_FILE, bytearray(filename))
        self.configMode = False

        if not ok:
            errst = 'communication failed!'
            return (errst, False)

        contents = str(bytearray(args))

        text = '<em>File ' + filename + ' contents:</em><br/>\n'
        text += contents
        text += '\n<br/>\n'

        return (text, True)


    def getFileListHTML(self, motename):
        (errst, ok) = self.checkValid()
        if not ok:
            return (errst, False)

        self.configMode = True
        (args, ok) = self.wmpExchangeCommand(WMP_CMD_GET_FILELIST, [])
        self.configMode = False

        if not ok:
            errst = 'communication failed!'
            return (errst, False)

        namelist = str(bytearray(args)).split('\n')

        motename = "mote" + motename

        if len(namelist) == 0 or len(namelist[0]) == 0:
            text = "The SD card is empty!"
        else:
            text = '<strong>Files:</strong><br/>\n'
            text += '<div class="files">'
            for filename in namelist:
                text += '<div class="entry"><span class="column1">'
                filename = fatFilenamePrettify(filename)
                text += filename
                text += '</span><span class="column2">'
                text += '&nbsp;&nbsp;<a href="config?filename=' + filename + '&' \
                    + motename + '_files=1&sel_' + motename + '=' + self.activePlatform.name \
                    + '">File contents</a>'
                text += "</span></div>\n"
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
        text += 'Port: <em>' + self.mote.getPortName() + '</em><br/>\n'
        text += 'Platform: <em>' + self.activePlatform.name + '</em><br/>\n'
        text += '</div>\n'

        # sensors
        text += '<strong>Sensor reading periods (in milliseconds, "0" means disabled):</strong><br/>\n'
        text += '<div class="subcontents">\n'
        for s in self.activePlatform.sensors:
            text += '<div class="entry">'
            text += utils.toTitleCase(s.name) + ": "
            text += '<input type="text" name="' + s.varname + '" value="' + str(s.period) + '"/><br/></div>\n'
        text += '</div>\n'

        # outputs
        text += '<strong>System outputs:</strong><br/>\n'
        text += '<div class="subcontents">\n'
        for s in self.activePlatform.outputs:
            text += '<div class="entry">'
            text += '<input type="checkbox" name="' + s.varname + '"'
            if s.isSelected: text += ' checked="checked"'
            text += '/> Write to ' + utils.toCamelCase(s.name)
            if s.varname == "file":
                text += '<br/><label for="filename">Filename: </label>'
                text += '<input type="input" name="filename" value="' + self.filenameOnMote + '"'
                text += ' title="The file will be created on mote\'s SD card" />\n'
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
        motename = "mote" + os.path.basename(self.mote.getPortName())
        text += '<input type="hidden" name="' + motename + '_cfg" value="1"/>\n'
        text += '<input type="hidden" name="sel_' + motename \
            + '" value="' + self.activePlatform.name + '" />\n'
        text += '<input type="submit" name="get" title="Read the currently active configuration values from the mote" value="Get values"/> \n'
        text += '<input type="submit" name="set" title="Set the web configuration values as active" value="Set values"/>\n'
        text += '</div></form>\n'
        return (text, True)

# global variable
instance = MoteConfig()
