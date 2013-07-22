#
# MansOS web server - server-side configuration settings
#

HTTP_SERVER_PORT = 30000
SERIAL_BAUDRATE = 38400

import configfile

# global variable
c = configfile.ConfigFile("server.cfg", automaticSections = True)

# default values
c.setCfgValue("port", HTTP_SERVER_PORT)
c.setCfgValue("baudrate", SERIAL_BAUDRATE)
c.setCfgValue("motes", [])
c.setCfgValue("selectedMotes", [])
 # in format <port>:<platform>, e.g. /dev/ttyUSB0:telosb
c.setCfgValue("motePlatforms", [])
c.setCfgValue("isSealCode", False)
c.setCfgValue("saveToFilename", "")
c.setCfgValue("saveToFilenameOnMote", "")
c.setCfgValue("saveProcessedData", False)
c.setCfgValue("slowUpload", False)
c.setCfgValue("htmlDirectory", "html")
c.setCfgValue("dataDirectory", "data")
c.setCfgValue("mansosDirectory", "../..")
c.setCfgValue("sealBlocklyDirectory", "seal-blockly")
c.setCfgValue("createDaemon", False)
c.setCfgValue("serverTheme", "simple")
c.setCfgValue("serverWebSettings", ["serverTheme"])
c.setCfgValue("serverSettingsType", ["[simple, green]"])
# database config
c.selectSection("database")
c.setCfgValue("dbName", "mansosdb")
c.setCfgValue("dbUsername", "root")
c.setCfgValue("dbPassword", "ln29Tx")
c.setCfgValue("dbHost", "localhost")
c.setCfgValue("senseApiKey", "cJ4Dm_Qb-3stWTWxCJgiFQ")
c.setCfgValue("senseApiFeeds", "light:37012,humidity:37013,temperature:37014")
c.setCfgValue("saveToDB", False)
c.setCfgValue("sendToOpensense", False)
# user config
c.selectSection("user")
c.setCfgValue("userDirectory", "user")
c.setCfgValue("userFile", "user.dat")
c.setCfgValue("userAttributes", ["name", "password", "level"])
c.setCfgValue("userAttributesType", ["text", "text", "[1, 9]"])
c.setCfgValue("defaultValues", ["unknown", "5f4dcc3b5aa765d61d8327deb882cf99", "1"]) #password "password"
c.setCfgValue("adminValues", ["admin", "21232f297a57a5a743894a0e4a801fc3", "9"]) #password "admin"
c.setCfgValue("userWebAttributes", []) #user editable (password is built-in)
c.setCfgValue("adminWebAttributes", ["level"]) #admin editable (reset password is built-in and name is uneditable)
# graph config
c.selectSection("graph")
c.setCfgValue("graphTitle", "Measurements_from_all_motes")
c.setCfgValue("graphYAxis", "Measurements")
c.setCfgValue("graphInterval", 1000)
c.setCfgValue("graphData", ["[all]"])
c.setCfgValue("graphAttributes", ["graphTitle", "graphYAxis", "graphInterval", "graphData"])

# load the config file
try:
    c.load()
except Exception as e:
    print("Failed to load configuration:")
    print(e)
    pass # let it be...

