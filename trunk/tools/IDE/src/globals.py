'''
Created on 16.01.2012.

@author: Janis Judvaitis
'''

# Global constants and parameters goes here!
# Note that this file is included in every project file.

# logging levels
ALL = 4
INFO = 3
WARNING = 2
ERROR  = 1
NO_LOGGING = 0
# Log texts
LOG_TEXTS = {
    INFO: "Info",
    WARNING: "Warning",
    ERROR: "Error"
}
# Set log level
LOG = ALL
# Allow log output to file
LOG_TO_FILE = True
# File name for log output
LOG_FILE_NAME = ".log"
# Allow log output to console
LOG_TO_CONSOLE = True

# Setting file name
SETTING_FILE = ".SEAL"

# Project types
SEAL_PROJECT = 0
MANSOS_PROJECT = 1

# Statement types
STATEMENT = 0
CONDITION = 1
STATE = 2
CONSTANT = 3
UNKNOWN = 4

# Upload targets
SHELL = 0
USB = 1