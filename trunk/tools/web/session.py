#
# MansOS web server - server-side session
#
import datetime, random, md5

alphabet = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,!?() '_-=+*/@$:%^#;~{}[]|`"
#nevar but simbols:"&<>"
lalphabet = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@_-.,`"
tabuList = ["admin"]

# --------------------------------------------
class Session():
    def __init__(self, sma):
        self._sma = sma
        self._oldsma = "0"
        self._end = datetime.datetime.now() + datetime.timedelta(minutes = 1)
        self._ode = False
    def add_sid(self, sid, user):
        self._sid = sid
        self._user = user
        print("{} had loged in".format(self._user["name"]))
    def get_all_data(self):
        temp = {}
        temp["sma"] = self._sma
        temp["end"] = self._end
        if hasattr(self, '_sid'):
            temp["sid"] = self._sid
        if hasattr(self, '_user'):
            temp["user"] = self._user
        return temp
    def del_sid(self):
        if hasattr(self, '_sid'):
            del self._sid
        if hasattr(self, '_user'):
            print("{} had loged out".format(self._user["name"]))
            del self._user
    def to_code(self, text, span = True, cod = "0"):
        if cod == "0":
            cod = str(random.randint(10000000, 99999999))
        if not hasattr(self, '_sid'):
            return False
        sid = self._sid
        alen = len(alphabet)
        tlen = len(text)
        while tlen % 32 != 0:
            poz = random.randint(0, tlen)
            text = text[:poz] + "`" + text[poz:]
            tlen += 1
        m = md5.new()
        m.update(sid + self._sma + cod)
        kript = m.hexdigest()
        ntext = ""
        i = 0
        while tlen > i:
            z = alphabet.find(text[i])
            if z > -1:
                z = z - int(kript[i % len(kript)] + kript[(i + 1) % len(kript)], 16)
                z = z % alen
                ntext += alphabet[z]
            else:
                ntext += text[i]
            i += 1
        if span:
            ntext = "<span class='coded' id='" + cod + "'>" + ntext + "</span>"
        return ntext
    def from_code(self, text): #code use to get coded information
        if not hasattr(self, '_sid'):
            return False
        sid = self._sid
        alen = len(lalphabet)
        tlen = len(text)
        m = md5.new()
        m.update(sid + self._oldsma)
        kript = m.hexdigest()
        ntext = ""
        i = 0
        while tlen > i:
            z = lalphabet.find(text[i])
            if z > -1:
                z = z + int(kript[i % len(kript)] + kript[(i + 1) % len(kript)], 16)
                z = z % alen
                if not lalphabet[z] == "`":
                    ntext += lalphabet[z]
            else:
                ntext += text[i]
            i += 1
        return ntext
    def to_md5(self, text): #md5 use to check coded infromation
        if not hasattr(self, '_sid'):
            return False
        m = md5.new()
        m.update(self._sid + self._oldsma + text)
        return m.hexdigest()

class Sessions():
    def __init__(self):
        self._sessionList = []
    def is_session(self, sma):
        i = self._sessionList.__len__()-1
        while -1 < i:
            if self._sessionList[i]._sma == sma:
                return True
            i -= 1
        return False
    def add_session(self, sma):
        self.delete_old()
        if self._sessionList.__len__() > 9999:
            print("Session count : {}".format(self._sessionList.__len__()))
            return
        self._sessionList.append(Session(sma))
        print("Session count : {}".format(self._sessionList.__len__()))
        return True
    def get_session(self, sma):
        i = self._sessionList.__len__() - 1
        while -1 < i:
            if self._sessionList[i]._sma == sma:
                return self._sessionList[i]
            i -= 1
        return False
    def get_session_old(self, oldsma):
        i = self._sessionList.__len__() - 1
        while -1 < i:
            if self._sessionList[i]._oldsma == oldsma:
                return self._sessionList[i]
            i -= 1
        return False
    def delete_old(self):
        i = self._sessionList.__len__() - 1
        while -1 < i:
            if self._sessionList[i]._end < datetime.datetime.now():
                if hasattr(self._sessionList[i], '_user'):
                    print("{} session ended".format(self._sessionList[i]._user["name"]))
                self._sessionList.pop(i)
            i -= 1
    def del_session(self, sma):
        i = self._sessionList.__len__() - 1
        while -1 < i:
            if self._sessionList[i]._sma == sma:
                if hasattr(self._sessionList[i], '_user'):
                    print("{} session ended".format(self._sessionList[i]._user["name"]))
                self._sessionList.pop(i)
            i -= 1
        
    def set_sma(self, osma, nsma):
        temp = self.get_session(osma)
        if temp:
            if nsma[-1:] != "0":
                temp._end = datetime.datetime.now() + datetime.timedelta(minutes = 15)
            else:
                temp._end = datetime.datetime.now() + datetime.timedelta(minutes = 1)
            temp._oldsma = temp._sma
            temp._sma = nsma
            return True
        else:
            nsma = nsma[:-1]+"0"
            self.add_session(nsma)
            return False
    def add_sid(self, sma, sid, user):
        i = self._sessionList.__len__() - 1
        while -1 < i:
            if self._sessionList[i]._sma == sma:
                self._sessionList[i].add_sid(sid, user)
                return
            i -= 1
    def get_sid(self, sma):
        i = self._sessionList.__len__() - 1
        while -1 < i:
            if self._sessionList[i]._sma == sma:
                return self._sessionList[i]._sid
            i -= 1
        return False
    def del_sid(self, sma):
        i = self._sessionList.__len__() - 1
        while -1 < i:
            if self._sessionList[i]._sma == sma:
                self._sessionList[i].del_sid()
                return
            i -= 1
    def get_sessions(self):
        temp = {}
        i = self._sessionList.__len__() - 1
        while -1 < i:
            temp[i] = self._sessionList[i].get_all_data()
            i -= 1
        return temp
 #-----------------------------------------
class setAndServeSessionAndHeader():
    def getCookie(self, cookieName):
        tdict = {}
        if "Cookie" in self.headers:
            tlist = self.headers['Cookie'].split(";")
            for i in tlist:
                i = i.strip()
                i = i.split("=")
                if len(i) == 2:
                    tdict[i[0]] = i[1]
            return tdict.get(cookieName, False)
        return False
        
    def changeHeadersCookie(self, cookieName, value):
        if "Cookie" in self.headers:
            if cookieName in self.headers["Cookie"]:
                tlist = self.headers["Cookie"].split(cookieName)
                if value == "":
                    tlist[1] = tlist[1][tlist[1].find(";"):]
                else:
                    tlist[0] += cookieName + "="
                    tlist[1] = value + tlist[1][tlist[1].find(";"):]
                self.headers["Cookie"] = tlist[0] + tlist[1]
            else:
                if value != "":
                    self.headers["Cookie"] += "; " + cookieName + "=" + value
        else:
            if value != "":
                self.headers["Cookie"] = cookieName + "=" + value
    def setSafe(self, state):
        self.headers["Safe"] = state
        return
    def isSafe(self):
        if "Safe" in self.headers:
            if self.headers["Safe"] == "True":
                return True
        return False
    def setSession(self, qs):
        csma = self.getCookie("Msma37")
        if csma:
            if not "sma" in qs:
                qs["sma"] = []
                qs["sma"].append(csma)
            else:
                qs["sma"][0] = csma
        csid=self.getCookie("Msid37")
        if csid:
            if not "sid" in qs:
                qs["sid"] = []
                qs["sid"].append(csid)
            else:
                qs["sid"][0] = csid
        tsma = str(random.randint(100000000, 999999999))
        self.setSafe("False")
        if "sma" in qs:
            tsma = tsma + "0"
            tses = self.sessions.get_session(qs["sma"][0])
            if tses and hasattr(tses, "_user"):
                if self.users.get_user("name", tses._user["name"]):
                    tses._user = self.users.get_user("name", tses._user["name"])
                    if "level" in tses._user:
                        tsma = tsma[:-1] + tses._user["level"]
                    else:
                        tsma = tsma[:-1] + "1"
                else:
                    qs["log"] = "out"
            if "log" in qs:
                if qs["log"] == "in":
                    tsid = random.randint(10000000000, 99999999999)
                    tuser = self.users.get_user("name", qs["user"][0])
                    if "level" in tuser:
                        tsma = tsma[:-1]+tuser["level"]
                    else:
                        tsma = tsma[:-1]+"1"
                    if qs["sma"][0][:2].isdigit():
                        fi = int(qs["sma"][0][:2])
                    else:
                        fi = 0
                    fi = fi%23
                    tcod = int(tuser["password"][fi:fi+9], 16)
                    qs["tsid"] = tsid - tcod
                    tsid = str(tsid)
                    msid = md5.new()
                    msid.update(tsid)
                    tsid = msid.hexdigest()
                    self.sessions.add_sid(qs["sma"][0], tsid, tuser)
                    ms = md5.new()
                    ms.update(tsid + qs["sma"][0])
                    tsid = ms.hexdigest()
                    self.changeHeadersCookie("Msid37", tsid)
                    if not "sid" in qs:
                        qs["sid"] = []
                        qs["sid"].append(tsid)
                    else:
                        qs["sid"][0] = tsid
                elif qs["log"] == "out":
                    tsma = tsma[:-1]+"0"
                    qs["del"] = "yes"
                    self.sessions.del_sid(qs["sma"][0])
                    if "sid" in qs:
                        del qs["sid"]
                    self.changeHeadersCookie("Msid37", "")
            if "sid" in qs:
                tses = self.sessions.get_session(qs["sma"][0])
                if tses:
                    if hasattr(tses, "_sid"):
                        m = md5.new()
                        m.update(tses._sid + qs["sma"][0])
                        if m.hexdigest() == qs["sid"][0]:
                            self.setSafe("True")
                        else:
                            self.setSafe("False")
                            print("Possible security intrusions attempted!")
                            self.sessions.del_session(tses._sma)
            if not self.sessions.set_sma(qs["sma"][0], tsma):
                tsma = tsma[:-1] + "0"
                self.changeHeadersCookie("Msid37", "")
                if "sid" in qs:
                     del qs["sid"]
                self.setSafe("False")
                qs["del"] = "yes"
        else:
            tsma = tsma + "0"
            self.sessions.add_session(tsma)
            qs["del"] = "yes"
        self.changeHeadersCookie("Msma37", tsma)
        if not "sma" in qs:
            qs["sma"] = []
            qs["sma"].append(tsma)
        else:
            qs["sma"][0] = tsma
        #print("This session is safe {}".format(self.isSafe()))
        #print("allSessions = ")
        #print(self.sessions.get_sessions()
            
    def serveSession(self, qs, urlTo):
        with open(self.htmlDirectory + "/session.html", "r") as f:
            contents = f.read()
            if "sma" in qs:
                if "log" in qs:
                    if qs["log"] == "in" and "tsid" in qs:
                        contents = contents.replace("%SID%", str(qs["tsid"]))
                        contents = contents.replace("/*?LOGIN", "")
                        
                contents = contents.replace("%RAND%", qs["sma"][0])
            if "del" in qs:
                if qs["del"] == "yes":
                    contents = contents.replace("/*?DEL", "")
            if urlTo != "":
                contents = contents.replace("%TO%", urlTo)
                contents = contents.replace("/*?REDIR", "")
                
            self.writeChunk(contents)
            
    def getLevel(self, qs = {}):
        if "sma" in qs:
            if qs["sma"][0][-1:].isdigit():
                return int(qs["sma"][0][-1:])
        csma = self.getCookie("Msma37")
        if csma:
            if csma[-1:].isdigit():
                return int(csma[-1:])
            return 0
        else:
            return 0
        
