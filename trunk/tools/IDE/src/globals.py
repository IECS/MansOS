# -*- coding: utf-8 -*-
#
# Copyright (c) 2008-2012 the MansOS team. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of  conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Global constants and parameters goes here!
# Note that this file is included in every project file.

# logging levels
LOG_ALL = 4
LOG_INFO = 3
LOG_WARNING = 2
LOG_ERROR = 1
LOG_OFF = 0
# Log texts
LOG_TEXTS = {
    LOG_INFO: "Info",
    LOG_WARNING: "Warning",
    LOG_ERROR: "Error"
}
# Set log level
LOG = LOG_ALL
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
END = 5

# Upload targets
SHELL = 0
USB = 1

# C keywords
C_KEYWORDS = ["void", "const", "struct", "char", "enum", "return",
              "uint8_t", "int8_t", "uint16_t", "int16_t", "int",
              "uint32_t", "int32_t", "bool",
              "if", "do", "while", "until", "for"]

# SEAL keywords
SEAL_KEYWORDS = ["and", "config", "const", "define", "else", "elsewhen", "end",
                 "load", "networkread", "not", "parameters", "pattern", "read",
                 "or", "output", "set", "use", "when", "where"]

# SEAL parameter names
SEAL_PARAMETERS = ['period', 'on', 'off', 'once', 'baudrate', 'id', 'cache',
                   'timestamp', 'crc', 'aggregate', 'address']
