#
# Configuration file parser (a wrapper around SafeConfigParser class)
#

# Note: characters "[]," are interpreted as meta-symbols and cannot be part of config values!

import ConfigParser

class ConfigFile(object):
    def __init__(self, filename, defaults = {}, automaticSections = False):
        self.cfg = ConfigParser.SafeConfigParser(defaults)
        self.currentSection = "main"
        self.filename = filename
        # do not load the file yet
        self.automaticSections = automaticSections

    def load(self):
        self.cfg.read(self.filename)

    def save(self):
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
        self.ensureCorrectSection(name)
        return self.cfg.get(self.currentSection, name)

    def getCfgValueAsInt(self, name):
        self.ensureCorrectSection(name)
        return self.cfg.getint(self.currentSection, name)

    def getCfgValueAsFloat(self, name):
        self.ensureCorrectSection(name)
        return self.cfg.getfloat(self.currentSection, name)

    def getCfgValueAsBool(self, name):
        self.ensureCorrectSection(name)
        return self.cfg.getboolean(self.currentSection, name)

    def getCfgValueAsList(self, name):
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
            return result
        # XXX: this means that comma cannot be part of well-formed config values!
        return value.split(",")

    def setCfgValue(self, name, value):
        # make sure the write is in correct section
        self.ensureCorrectSection(name)
        # make sure the value is in acceptable format (lists are stored as strings)
        if isinstance(value, list):
            value = ",".join(value)
        elif not isinstance(value, str):
            value = str(value)
        # make sure the selected section is present in the file
        if not self.cfg.has_section(self.currentSection):
            self.cfg.add_section(self.currentSection)
        # write the value
        self.cfg.set(self.currentSection, name, value)

