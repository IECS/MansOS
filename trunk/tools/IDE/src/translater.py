translations = { 
    "eng" :{
        "period":"Period",
        "on_time":"Turn on at",
        "off_time":"Turn off at",
        "turn_off":"Turn off",
        "turn_on":"Turn on",
        "blinkOnce":"Blink one time",
        "blinkTwice":"Blink 2 times",
        "blinkTimes":"Blink following times",
        "print":"Print",
        "aggregate":"Aggregate",
        "baudrate":"Baudrate",
        "crc":"Add checksum"
        }
}
def translate(data, lang = "eng"):
    if data in translations[lang]:
        return translations[lang][data]
    return data