
from __future__ import print_function
import md5

class PageLogin():
    def serveLogin(self, qs):
        csma = self.getCookie("Msma37")
        if csma:
            if not "sma" in qs:
                qs["sma"] = []
                qs["sma"].append(csma)
            else:
                qs["sma"][0] = csma
        qs["log"] = ""
        changes = {}
        changes["FAIL"] = ''
        if "sma" in qs and "0" != qs["sma"][0][-1:]:
            qs["log"] = "out"
            self.setSession(qs)
            self.serveDefault(qs, True)
            return
        elif not "sma" in qs and "password" in qs and "user" in qs:
            changes["FAIL"]="Could not identify connection!"
        elif "password" in qs and "user" in qs:
            tuser = self.users.get_user("name", qs["user"][0])
            if tuser:
                m = md5.new()
                m.update(tuser["password"]+qs["sma"][0])
                if tuser and m.hexdigest() == qs["password"][0]:
                    qs["log"] = "in"
                    self.setSession(qs)
                    self.serveDefault(qs, True)
                    return
            changes["FAIL"] = "You have made a mistake!"
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        
        content = self.serveBody("login", qs, changes)
        self.serveAnyPage("login", qs, content = content)
