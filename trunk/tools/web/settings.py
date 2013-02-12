#
# MansOS web server - server-side configuration settings
#

HTTP_SERVER_PORT = 30000
SERIAL_BAUDRATE = 38400

#
# Class that contains all global settings
#
class Settings(object):
    class ConfigValues(object):
        def __init__(self):
            #server.cfg
            self.port = str(HTTP_SERVER_PORT)
            self.baudrate = str(SERIAL_BAUDRATE)
            self.motes = []
            self.selectedMotes = []
            self.motePlatforms = [] # in format <port>:<platform>, e.g. /dev/ttyUSB0:telosb
            self.isSealCode = "False"
            self.saveToFilename = ""
            self.saveToFilenameOnMote = ""
            self.saveProcessedData = "False"
            self.saveMultipleFiles = "False"
            self.slowUpload = "False"
            self.htmlDirectory = "html"
            self.dataDirectory = "data"
            self.mansosDirectory = "../.."
            self.sealBlocklyDirectory = "seal-blockly"
            self.createDaemon = "False"
            #user.cfg
            self.userDirectory = "user"
            self.userFile = "user.dat"
            self.userAttributes = "name,password,level"
            self.defaultValues = "Unknown,password,1"
            self.adminValues = "admin,admin,9"

    cfg = ConfigValues()

    configurationFileName = "server.cfg"
    userconfigurationFileName = "user.cfg"

    def load(self):
        self.comments = {}
        tmpComment = ""

        for files in [self.configurationFileName, self.userconfigurationFileName]:
            with open(files, 'r') as f:
                line = 0
                for x in f.readlines():
                    line += 1
                    x = x.strip()
                    if x == '' or x[0] == '#': # skip comments and empty lines
                        tmpComment += x + '\r\n'
                        continue

                    # extract key=value
                    kv = x.split("=")
                    if len(kv) != 2:
                        print("Syntax error in configuration file line {}".format(line))
                        continue

                    key = kv[0]
                    if key not in self.cfg.__dict__:
                        print("Unknown configuration key " + key)
                        continue

                    if tmpComment:
                        self.comments[key] = tmpComment
                        tmpComment = ""

                    # extract value list
                    vv = kv[1].strip('"').split(",")

                    if len(vv) == 1:
                        # single value
                        self.cfg.__setattr__(key, vv[0])
                    else:
                        # value list
                        self.cfg.__setattr__(key, vv)

                if tmpComment:
                    self.comments["__EOF"] = tmpComment

    def save(self):
        with open(self.configurationFileName, 'w') as f:
            for key in self.cfg.__dict__:
                if key[0] == '_': continue  # skip generic and special attributes
                value = self.cfg.__dict__[key]
                comment = self.comments.get(key, "")
                if comment:
                    f.write(comment)
                f.write(key)
                f.write('=')
                if isinstance(value, list):
                    # value list
                    f.write(",".join(value))
                else:
                    # single value
                    f.write(value)
                f.write("\r\n")

            comment = self.comments.get("__EOF", "")
            if comment:
                f.write(comment)

    def getCfgValue(self, name):
        return self.cfg.__getattribute__(name)

    def getCfgValueAsInt(self, name, default = 0):
        try:
            result = int(self.cfg.__getattribute__(name), 0)
        except:
            result = default
        return result

    def getCfgValueAsBool(self, name, default = False):
        try:
            result = self.cfg.__getattribute__(name).lower() in ["true", "yes", "y", "1"]
        except:
            result = default
        return result

    def setCfgValue(self, name, value):
        if isinstance(value, list):
            self.cfg.__setattr__(name, value)
        else:
            self.cfg.__setattr__(name, str(value))


# global variable
settingsInstance = Settings()
try:
    settingsInstance.load()
except Exception as e:
    print("Failed to load configuration:")
    print(e)
    pass

def isascii(c, printable = True):
    if 0x00 <= ord(c) <= 0x7f:
        if 0x20 <= ord(c) <= 0x7e:
            return True
        if printable and (c == '\r' or c == '\n' or c == '\t'):
            return True
        return False
    else:
        return False


def isasciiString(s):
    for c in s:
        if not isascii(c, False):
            return False
    return True
