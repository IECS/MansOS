#
# Configuration file parser (a wrapper around SafeConfigParser class)
#

# Note: characters "[]," are interpreted as meta-symbols and cannot be part of config values!

import ConfigParser, threading

class ConfigFile(object):
    def __init__(self, filename, defaults = {}, automaticSections = False):
        self.cfg = ConfigParser.SafeConfigParser(defaults)
        self.currentSection = "main"
        self.filename = filename
        # do not load the file yet
        self.automaticSections = automaticSections
        self.lock = threading.Lock()

    def load(self):
        self.cfg.read(self.filename)

    def save(self):
        with self.lock:
            with open(self.filename, 'wb') as configfile:
                self.cfg.write(configfile)

    def selectSection(self, sectionName):
        self.currentSection = sectionName

    def ensureCorrectSection(self, optionName):
        if self.automaticSections and \
                not self.cfg.has_option(self.currentSection, optionName):
            for s in self.cfg.sections():
                if self.cfg.has_option(s, optionName):
                    self.currentSection = s
                    break

    def getCfgValue(self, name):
        result = ""
        with self.lock:
            self.ensureCorrectSection(name)
            result = self.cfg.get(self.currentSection, name)
        return result

    def getCfgValueAsInt(self, name):
        result = 0
        with self.lock:
            self.ensureCorrectSection(name)
            result = self.cfg.getint(self.currentSection, name)
        return result

    def getCfgValueAsFloat(self, name):
        result = 0.0
        with self.lock:
            self.ensureCorrectSection(name)
            result = self.cfg.getfloat(self.currentSection, name)
        return result

    def getCfgValueAsBool(self, name):
        result = False
        with self.lock:
            self.ensureCorrectSection(name)
            result = self.cfg.getboolean(self.currentSection, name)
        return result

    def getCfgValueAsList(self, name):
        result = []
        with self.lock:
            self.ensureCorrectSection(name)
            value = self.cfg.get(self.currentSection, name)
            # convert to list
            split_by_bracket = value.split("]")
            if len(split_by_bracket) > 1:
                # ok, got list in a list here
                result = []
                for s in split_by_bracket:
                    if len(s):
                        result.append(s.strip(",[").split(","))
            else:
                # XXX: this means that comma cannot be part of well-formed config values! 
                result = value.split(",")
        return result

    def getCfgValueAsRealList(self, name):
        result = []
        with self.lock:
            self.ensureCorrectSection(name)
            value = self.cfg.get(self.currentSection, name)
            try:
                import json
                result = json.loads(value)
            except ValueError:
                result = value.split(",")
        return result

    def setCfgValue(self, name, value):
        # make sure the value is in acceptable format (lists are stored as strings)
        if isinstance(value, list):
            value = ",".join(value)
        elif not isinstance(value, str):
            value = str(value)

        with self.lock:
            if self.cfg.has_section(self.currentSection):
                # make sure the write is in correct section
                self.ensureCorrectSection(name)
            else:
                # make sure the selected section is present in the file
                self.cfg.add_section(self.currentSection)
            # write the value
            self.cfg.set(self.currentSection, name, value)
