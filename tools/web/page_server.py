import random
class PageServer():
    def getServerSettingsType(self, attribute):
        ua = self.settings.getCfgValue("serverWebSettings")
        if isinstance(ua, str):
            ua = [ua]
        at = self.settings.getCfgValue("serverSettingsType")
        if isinstance(at, str):
            at = [at]
        i = len(ua) - 1
        while i > -1:
            if attribute == ua[i]:
                return at[i]
            i -= 1
        return "text"
    
    def serveServer(self, qs, isSession = False):
        if not isSession:
            self.setSession(qs)
        if not self.getLevel() > 0:
            self.serveDefault(qs, True)
            return
        changes = {}
        webSettings = self.settings.getCfgValue("serverWebSettings")
        if isinstance(webSettings, str):
            webSettings = [webSettings]
        tses = self.sessions.get_session(qs["sma"][0])
        changes["INFO"] = ''
        if "savesettings" in qs:
            #save changes
            if qs["savesettings"][0] == tses.to_md5("ServerSettingsSave") and self.isSafe():
                for setting in webSettings:
                    if not setting in qs:
                        continue
                    self.settings.setCfgValue(setting, tses.from_code(qs[setting][0]))
                changes["INFO"] = "<h4 class='suc'> Saved changes! </h4>"
                self.settings.save()
            else:
                changes["INFO"] = "<h4 class='err'> Wrong data for saving! </h4>"
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("server", qs)
        changes["FORM"] = ''
        if self.isSafe():
            if tses:
                formCode = "<form>"
                for setting in webSettings:
                    tcod = str(random.randint(10000000, 99999999))
                    ttype = self.getServerSettingsType(setting)
                    if type(ttype) is list:
                        formCode += "<p><input autocomplete='off' type='hidden' class='coded tocode list' id='" + tcod
                        formCode += "' value=\"" + tses.to_code(self.settings.getCfgValue(setting), False, tcod) + "\" name='" + setting + "'>"
                        formCode += setting +": <select id='" + setting + "'>"
                        for option in ttype:
                            formCode += "<option value=" + option + ">" + option + "</option>"
                        formCode += "</select></p>"
                    elif ttype == "bool":
                        formCode += "<p><input autocomplete='off' type='hidden' class='coded tocode bool' id='" + tcod
                        formCode += "' value=\"" + tses.to_code(self.settings.getCfgValue(setting), False, tcod) + "\" name='" + setting + "'>"
                        formCode += "<input type='checkbox' class='checkbox' id='" + setting + "'>" + setting + "</p>"
                    else:
                        formCode += "<p>" + setting + ": <input autocomplete='off' type='text' class='coded tocode' id='" + tcod
                        formCode += "' value=\"" + tses.to_code(self.settings.getCfgValue(setting), False, tcod) + "\" name='" + setting + "'></p>"
                #logging option
                formCode += "<p><input type='hidden' class='md5' id='ServerSettingsSave' name='savesettings'>"
                formCode += "<input type='submit' onclick='return settingsSave()' value='Save'></p>"
                formCode += "</form>"
                changes["FORM"] = formCode

        self.serveBody("server", qs, changes)
        self.serveFooter()

