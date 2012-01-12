# -*- coding: UTF-8 -*-
class Translater:
    def __init__(self):
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
                "blinkTimes": "Blink following times",
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
                "Open document": "Atvērt dokukmentu",
                "Open upload window": "Atvērt augšupielādes logu",
                "Open read output window": "Atvērt klausīšanās logu",
                "Exit application": "Aizvērt lietotni",
                "Upload and compile": "Augšupielādēšana un kompilēšana",
                "Listen to output": "Klausīties izvadu",
                "Changed platform to": "Platforma nomainīta uz ",
                "No devices found": "Nav atrasta neviena ierīce",
                "Use default device": "Izmantot noklusēto iecīri",
                "Starting compile...": "Kompilēšana sākta...",
                "Compiled successfully!": "Kompilēšana pabeigta veiksmīgi",
                "Starting upload...": "Augšupielādēšana sākta...",
                "Uploaded successfully!": "Augšupielādēšana pabeigta veiksmīgi",
                "Start listening": "Sākt klausīties",
                "Stop listening": "Beigt klausīties",
                "Change language": "Nomainīt valodu",
                "Add statement": "Pievienot darbību",
                "Add condition": "Pievienot nosacījumu"
                }
        }
        
        self.activeLanguage = "LV"
        
    def translate(self, data, lang = ''):
        if lang == '':
            lang = self.activeLanguage
        if data in self.translations[lang]:
            return self.translations[lang][data]
        return data