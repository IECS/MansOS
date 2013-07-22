#
# Remote access proxy server - configuration settings
#

HTTP_SERVER_PORT = 30001
SERIAL_BAUDRATE = 38400

import configfile

# global variable
c = configfile.ConfigFile("remoteaccess.cfg")

# default values
c.setCfgValue("port", HTTP_SERVER_PORT)
c.setCfgValue("baudrate", SERIAL_BAUDRATE)
c.setCfgValue("parity", "none")
c.setCfgValue("stopbits", "1")
c.setCfgValue("motes", [])
c.setCfgValue("mansosDirectory", "../..")
c.setCfgValue("slowUpload", False)
c.setCfgValue("createDaemon", False)

# load the config file
try:
    c.load()
except Exception as e:
    print("Failed to load configuration:")
    print(e)
    pass # let it be...
