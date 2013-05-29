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
            self.slowUpload = "False"
            self.htmlDirectory = "html"
            self.dataDirectory = "data"
            self.mansosDirectory = "../.."
            self.sealBlocklyDirectory = "seal-blockly"
            self.createDaemon = "False"
            self.serverTheme = "simple"
            self.serverWebSettings = ["serverTheme"]
            self.serverSettingsType = [["simple","green"]]
            self.dbServer = "mysql://%s:%s@%s/mansosdb"
            self.dbUsername = "root"
            self.dbPassword = "ln29Tx"
            self.dbHost = "localhost"
            #user.cfg
            self.userDirectory = "user"
            self.userFile = "user.dat"
            self.userAttributes = ["name", "password", "level"]
            self.userAttributesType = ["text", "text", ["1", "9"]]
            self.defaultValues = ["unknown","5f4dcc3b5aa765d61d8327deb882cf99","1"] #password "password"
            self.adminValues = ["admin","21232f297a57a5a743894a0e4a801fc3","9"] #password "admin"
            self.userWebAttributes = [] #user editable (password is built-in)
            self.adminWebAttributes = ["level"] #admin editable (reset password is built-in and name is uneditable)
            #graph.cfg
            self.graphTitle = "Measurements_from_all_motes"
            self.graphYAxis = "Measurements"
            self.graphInterval = "1000"
            self.graphData = [["all"]]
            self.graphMaxDisplay = "40"
            

    cfg = ConfigValues()

    FileNames = ["server.cfg", "user.cfg","graph.cfg"]
    _inFile = {}
    def listInList(self, alist):
        i = len(alist) - 1
        blist = [] #blist is end result
        while i >= 0:
            if alist[i][-1:] == "]":
                if alist[i][:1] == "[":
                    blist.insert(0, [alist[i][1:-1]])
                    i -= 1
                    continue
                j = i - 1
                clist = [alist[i][:-1]] #clist is list in list
                while j > -1:
                    if alist[j][:1] == "[":
                        clist.insert(0, alist[j][1:])
                        blist.insert(0, clist)
                        i = j
                        break
                    else:
                        clist.insert(0, alist[j])
                    j -= 1
                if j == -1: blist.insert(0, alist[i]) #no one befor start with "["
            else:
                blist.insert(0, alist[i])
            i -= 1
        if i == 0: blist.insert(0, alist[0])
            #element [0] isn't check if it end with 0, but could be added
        return blist
    def load(self):
        self.comments = {}
        tmpComment = ""
        
        for files in self.FileNames:
            with open(files, 'r') as f:
                line = 0
                self._inFile[files] = []
                for x in f.readlines():
                    line += 1
                    x = x.strip()
                    if x == '' or x[0] == '#': # skip comments and empty lines
                        tmpComment += x + '\r\n'
                        continue

                    # extract key=value
                    kv = x.split("=")
                    if len(kv) != 2:
                        print("Syntax error in configuration file {} line {}".format(files,line))
                        continue

                    key = kv[0]
                    if key not in self.cfg.__dict__:
                        print("Unknown configuration key " + key)
                        continue

                    if tmpComment:
                        self.comments[key] = tmpComment
                        tmpComment = ""
                    self._inFile[files].append(key)

                    # extract value list
                    vv = kv[1].strip('"').split(",")

                    if len(vv) == 1 and (len(vv[0]) < 2 or (vv[0][:1] != "[" and vv[0][-1:] != "]")):
                        # single value
                        self.cfg.__setattr__(key, vv[0])
                    else:
                        # value list
                        vv = self.listInList(vv)
                        self.cfg.__setattr__(key, vv)

                if tmpComment:
                    self.comments["__EOF"] = tmpComment

    def save(self, setting = "all"):#save all file with given setting, "all" - saving all files
        if setting == "all":
            tfiles = self._inFile.keys()
        else:
            tfiles = []
            for files in self._inFile.keys():
                for key in self._inFile[files]:
                    if setting == key:
                        tfiles.append(files)
        for files in tfiles:
            with open(files, 'w') as f:
                for key in self._inFile[files]:
                    value = self.cfg.__dict__[key]
                    comment = self.comments.get(key, "")
                    if comment:
                        f.write(comment)
                    f.write(key)
                    f.write('=')
                    if isinstance(value, list):
                        # value list
                        first = True
                        for values in value:
                            if not first: f.write(",")
                            if isinstance(values, list):
                                f.write("[" + ",".join(values) + "]")
                            else:
                                f.write(values)
                            first = False
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
