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

from globals import * #@UnusedWildImport

class Translater:
    def __init__(self, parent):
        self.parent = parent
        self.translations = {
            "ENG" :{
                'langName':"English",

                # SEAL SPECIFIC PART
                "period": "Period",
                "on_at": "Turn on at",
                "off_at": "Turn off at",
                "turn_off": "Turn off",
                "turn_on": "Turn on",
                "blink": "Blink one time",
                "blinkTwice": "Blink 2 times",
                "blinkTimes": "Blink n times",
                "print": "Print",
                "aggregate": "Aggregate",
                "baudrate": "Baudrate",
                "crc": "Add checksum",
                # APLICATION SPECIFIC PART
                # is usually written in English, so no need to translate
                },

            "LV" :{
                'langName':"Latviešu",

                # SEAL SPECIFIC PART
                "period": "Periods",
                "on_at": "Ieslēgt pēc",
                "off_at": "Izslēgt pēc",
                "turn_off": "Izslēgt",
                "turn_on": "Ieslēgt",
                "blink": "Mirgot vienu reizi",
                "blinkTwice": "Mirgot 2 reizes",
                "blinkTimes": "Mirgot n reizes",
                "print": "Izvadīt",
                "aggregate": "Apvienot",
                "baudrate": "Baudrate",
                "crc": "Kontrolsumma",
                "When": "When",
                "Elsewhen": "Elsewhen",

                # APLICATION SPECIFIC PART
                "File": "Izvēlne",
                "Options": "Uzstādījumi",
                "New": "Jauns",
                "Save": "Saglabāt",
                "Save as": "Saglabāt kā",
                "Reload": "Pārlādēt",
                "Open": "Atvērt",
                "Upload": "Augšupielādēt",
                "Read output": "Klausīties",
                "Exit": "Iziet",
                "Close": "Aizvērt",
                "Edit": "Labot",
                "files": "datnes",
                "Compile": "Kompilēt",
                "Refresh": "Atsvaidzināt",
                "Exception": "Izņēmumsituācija",
                "Edit condition": "Labot nosacījumu",
                "Untitled document": "Nenosauks dokuments",
                "Unsaved document": "Nesaglabāts dokuments",
                "All files": "Visas datnes",
                "Open new document": "Atvērt jaunu dokumentu",
                "Save changes to": "Saglabāt izmaiņas dokumentā",
                "before close it?": "pirms to aizvērt?",
                "Create empty document": "Izveidot tukšu dokumentu",
                "Save document": "Saglabāt dokumentu",
                "Save document as": "Saglabāt dokumentu kā",
                "Open document": "Atvērt dokumentu",
                "Open upload window": "Atvērt augšupielādes logu",
                "Open read output window": "Atvērt klausīšanās logu",
                "Exit application": "Aizvērt lietotni",
                "Upload and compile": "Augšupielādēšana un kompilēšana",
                "Listen to output": "Klausīties izvadu",
                "Changed platform to": "Platforma nomainīta uz ",
                "No devices found": "Nav atrasta neviena ierīce",
                "Use default device": "Izmantot ierīci pēc noklusēšanas",
                "Starting compile": "Kompilēšana sākta",
                "Compiled successfully in": "Kompilēšana pabeigta veiksmīgi, laiks:",
                "Starting upload on": "Sāku augsupielādēt uz ",
                "Uploaded successfully in": "Augšupielādēšana pabeigta veiksmīgi, laiks:",
                "Upload failed with message:": "Augšupielādesana neizdevās, kļūda:",
                "Start listening": "Sākt klausīties",
                "Stop listening": "Beigt klausīties",
                "Change language": "Nomainīt valodu",
                "Add statement": "Pievienot darbību",
                "Add condition": "Pievienot nosacījumu",
                "Edit actuator": "Labot darbību",
                "Edit object": "Labot objektu",
                "Got": "Atradu",
                "devices in": "iekārtas, laiks:",
                "default device": "ierīces pēc noklusēšanas",
                "Searching devices": "Meklēju ierīces",
                "No devices found!" : "Ierīces nav atrastas!",
                "Information": "Informācija",
                "Listen": "Klausīties",
                "Configure upload and compile": "Konfigurēt augsupielādēšanu un kompilēšanu"
                }
        }

    def translate(self, data, lang = ''):
        if lang == '':
            lang = self.parent.getSetting("activeLanguage")
        # This happens when no language is set
        if lang != '':
            if data in self.translations[lang]:
                return self.translations[lang][data]
        # Don't log ENG, because it's not ment to be translated
        if lang != "ENG":
            self.parent.logMsg(LOG_WARNING, "No translation for " +
                             "'" + lang + "': '" + data + "'")
        return data
