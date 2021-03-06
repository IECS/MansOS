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


import ConfigParser

class Settings:
    configFile = "config.cfg"

    @staticmethod
    def __init__():
        defaultSettings = {
            "active_language" : "ENG",
            "platform" : "telosb",
            "blockly_location": "../../../seal-blockly/blockly/demos/seal/index.html",
            "blockly_port" : '8090',
            "blockly_host" : "localhost",
            "recently_opened_count" : "10"
        }
        Settings.config = ConfigParser.SafeConfigParser(defaultSettings)
        Settings.config.read(Settings.configFile);

        if not Settings.config.has_section("CURRENT"):
            Settings.config.add_section("CURRENT")

    @staticmethod
    def saveConfig():
        with open(Settings.configFile, 'wb') as file:
            Settings.config.write(file)

    @staticmethod
    def get(name):
        try:
            return Settings.config.get("CURRENT", name)
        except:
            print "No config entry found: " + name
            return ""

    @staticmethod
    def set(name, value):
        try:
            return Settings.config.set("CURRENT", name, str(value))
        except:
            print "Can't add config(" + name + " : " + value + ")"