# -*- coding:utf-8 -*-
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
                'langName': "Latviesu", # why 'š' creates fail???

                # SEAL SPECIFIC PART
                "period": u"Periods",
                "on_at": u"Ieslēgt pēc",
                "off_at": u"Izslēgt pēc",
                "turn_off": u"Izslēgt",
                "turn_on": u"Ieslēgt",
                "blink": u"Mirgot vienu reizi",
                "blinkTwice": u"Mirgot 2 reizes",
                "blinkTimes": u"Mirgot n reizes",
                "print": u"Izvadīt",
                "once": u"Vienreiz",
                "aggregate": u"Apvienot",
                "baudrate": u"Baudrate",
                "crc": u"Kontrolsumma",
                "When": u"When",
                "Elsewhen": u"Elsewhen",

                # APLICATION SPECIFIC PART
                "File": u"Izvēlne",
                "Options": u"Uzstādījumi",
                "New": u"Jauns",
                "Save": u"Saglabāt",
                "Save as": u"Saglabāt kā",
                "Reload": u"Pārlādēt",
                "Examples": u"Piemēri",
                "Open": u"Atvērt",
                "Upload": u"Augšupielādēt",
                "Information": u"Informācija",
                "Listen": u"Klausīties",
                "Exit": u"Iziet",
                "Close": u"Aizvērt",
                "Edit": u"Labot",
                "files": u"datnes",
                "Compile": u"Kompilēt",
                "Refresh": u"Atsvaidzināt",
                "Exception": u"Izņēmumsituācija",
                "Edit condition": u"Labot nosacījumu",
                "Untitled document": u"Nenosauks dokuments",
                "Unsaved document": u"Nesaglabāts dokuments",
                "All files": u"Visas datnes",
                "Open new document": u"Atvērt jaunu dokumentu",
                "Save changes to": u"Saglabāt izmaiņas dokumentā",
                "before close it?": u"pirms to aizvērt?",
                "Create empty document": u"Izveidot tukšu dokumentu",
                "Save document": u"Saglabāt dokumentu",
                "Save document as": u"Saglabāt dokumentu kā",
                "Open document": u"Atvērt dokumentu",
                "Open upload window": u"Atvērt augšupielādes logu",
                "Open read output window": u"Atvērt klausīšanās logu",
                "Exit application": u"Aizvērt lietotni",
                "Upload and compile": u"Augšupielādēšana un kompilēšana",
                "Listen to output": u"Klausīties izvadu",
                "Changed platform to": u"Platforma nomainīta uz ",
                "No devices found": u"Nav atrasta neviena ierīce",
                "Use default device": u"Izmantot ierīci pēc noklusēšanas",
                "Starting compile": u"Kompilēšana sākta",
                "Compiled successfully in": u"Kompilēšana pabeigta veiksmīgi, laiks:",
                "Starting upload on": u"Sāku augsupielādēt uz ",
                "Uploaded successfully in": u"Augšupielādēšana pabeigta veiksmīgi, laiks:",
                "Upload failed with message:": u"Augšupielādesana neizdevās, kļūda:",
                "Start listening": u"Sākt klausīties",
                "Stop listening": u"Beigt klausīties",
                "Change language": u"Nomainīt valodu",
                "Add statement": u"Pievienot darbību",
                "Add condition": u"Pievienot nosacījumu",
                "Edit actuator": u"Labot darbību",
                "Edit object": u"Labot objektu",
                "Got": u"Atradu",
                "devices in": u"iekārtas, laiks:",
                "the default device": u"ierīces pēc noklusēšanas",
                "Searching devices": u"Meklēju ierīces",
                "No devices found!" : u"Ierīces nav atrastas!",
                "Configure upload and compile": u"Konfigurēt kompilēšanu un augšupielādēšanu",
                "Untitled": u"Nenosaukts",
                "Recently used files": u"Nesen lietotās datnes",
                "Add window": u"Pievienot logu",
                "Show window": u"Parādīt logu",
                "Add listen window": u"Pievienot klausīšanās logu",
                "Show blockly window": u"Pievienot Blockly logu",
                "Show edit window": u"Pievienot rediģēšanas logu",
                "Seal documentation": u"Seal dokumentācija",
                "About": u"Par",
                "MansOS documentation": u"MansOS dokumentācija",
                "Windows": u"Logi",
                "Help": u"Palīdzība",
                "Listen node's output": u"Klausīties sensoru mezgla izvadu",
                "Editors": u"Redaktori",
                "Info": u"Informācija",
                "Listen module": u"Klausīšanās modulis",
                "Visual edit": u"Vizuālā rediģēšana",
                "Add mote": u"Pievienot sensoru mezglu",
                "No device found on this port": u"Šajā portā nav atrasta neviens sensoru mezgls",
                "There already is device on that port in list": u"Sensoru mezgls ar šādu portu jau atrodas sensoru mezglu sarakstā",
                "Mote name": u"Sensoru mezgla nosaukums",
                "Add new mote": u"Pievienot jaunu sensoru mezglu",
                "Mote port": u"Sensoru mezgla ports"
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
