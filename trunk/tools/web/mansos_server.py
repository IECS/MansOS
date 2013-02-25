#!/usr/bin/env python

#
# MansOS web server - main file
#

import os, sys, platform, datetime, cookielib, random, md5
import threading, time, serial, select, socket, cgi, subprocess, struct, signal
import json
from settings import *
from mote import *
from sensor_data import *
from config import *
from daemon import *

def isPython3():
    return sys.version_info[0] >= 3

if isPython3():
    from http.server import *
    from socketserver import *
    from urllib.parse import *
else:
    from BaseHTTPServer import *
    from SocketServer import *
    from urlparse import *

isListening = False
listenThread = None

lastUploadCode = ""
lastUploadConfig = ""
lastUploadFile = ""
lastJsonData = ""

isInSubprocess = False
uploadResult = ""

motes = MoteCollection()

htmlDirectory = "html"
sealBlocklyPath = "seal-blockly"
alphabet = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,!?() '_-=+*/@$:%^#;~{}[]|`" #nevar but simbols:"&<>"
lalphabet = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ`"
tabuList = ["admin"]

# --------------------------------------------
class Session():
    def __init__(self,sma):
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
                z = z - int(kript[i % len(kript)] + kript[(i + 1) % len(kript)],16)
                z = z % alen
                ntext += alphabet[z]
            else:
                ntext += text[i]
            i += 1
        if span:
            ntext= "<span class='coded' id='" + cod + "'>" + ntext + "</span>"
        return ntext
    def from_code(self, text): #code lieto ja teksts ir mainigs
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
                z = z + int(kript[i % len(kript)] + kript[(i + 1) % len(kript)],16)
                z = z % alen
                if not lalphabet[z] == "`":
                    ntext += lalphabet[z]
            else:
                ntext += text[i]
            i += 1
        return ntext
    def to_md5(self, text): #md5 lietoja, ja teksts ir nemainigs
        if not hasattr(self, '_sid'):
            return False
        m = md5.new()
        m.update(self._sid + self._oldsma + text)
        return m.hexdigest()

class Sessions():
    def __init__(self):
        self._sessionList = []
    def is_session(self,sma):
        i=self._sessionList.__len__()-1
        while -1 < i:
            if self._sessionList[i]._sma == sma:
                return True
            i -=1
        return False
    def add_session(self,sma):
        self.delete_old()
        if self._sessionList.__len__()>9999:
            print("Session count : {}".format(self._sessionList.__len__()))
            return
        self._sessionList.append(Session(sma))
        print("Session count : {}".format(self._sessionList.__len__()))
        return True
    def get_session(self,sma):
        i=self._sessionList.__len__()-1
        while -1 < i:
            if self._sessionList[i]._sma == sma:
                return self._sessionList[i]
            i -=1
        return False
    def delete_old(self):
        i=self._sessionList.__len__()-1
        while -1 < i:
            if self._sessionList[i]._end < datetime.datetime.now():
                if hasattr(self._sessionList[i], '_user'):
                    print("{} session ended".format(self._sessionList[i]._user["name"]))
                self._sessionList.pop(i)
            i -=1
    def del_session(self,sma):
        i=self._sessionList.__len__()-1
        while -1 < i:
            if self._sessionList[i]._sma == sma:
                if hasattr(self._sessionList[i], '_user'):
                    print("{} session ended".format(self._sessionList[i]._user["name"]))
                self._sessionList.pop(i)
            i -=1
        
    def set_sma(self,osma,nsma):
        temp = self.get_session(osma)
        if temp:
            if nsma[-1:] != "0":
                temp._end=datetime.datetime.now() + datetime.timedelta(minutes=15)
            else:
                temp._end=datetime.datetime.now() + datetime.timedelta(minutes=1)
            temp._oldsma=temp._sma
            temp._sma=nsma
            return True
        else:
            nsma=nsma[:-1]+"0"
            self.add_session(nsma)
            return False
    def add_sid(self,sma,sid,user):
        i=self._sessionList.__len__()-1
        while -1 < i:
            if self._sessionList[i]._sma == sma:
                self._sessionList[i].add_sid(sid,user)
                return
            i -=1
    def get_sid(self,sma):
        i=self._sessionList.__len__()-1
        while -1 < i:
            if self._sessionList[i]._sma == sma:
                return self._sessionList[i]._sid
            i -=1
        return False
    def del_sid(self,sma):
        i=self._sessionList.__len__()-1
        while -1 < i:
            if self._sessionList[i]._sma == sma:
                self._sessionList[i].del_sid()
                return
            i -=1
    def get_sessions(self):
        temp = {}
        i=self._sessionList.__len__()-1
        while -1 < i:
            temp[i] = self._sessionList[i].get_all_data()
            i -=1
        return temp
        
# --------------------------------------------
class User():
    def __init__(self, userAttributes, data):
        self._attributes = {}
        i=0
        while data.__len__() > i:
            self._attributes[userAttributes[i]] = data[i]
            i+=1
    def set_attributes(self, key, value):
        self._attributes[key] = value
    def get_data(self, key):
        return self._attributes.get(key, False)
    def get_all_data(self):
        return self._attributes
    
class Users():
    def __init__(self, userAttr):
        self._userAttributes = userAttr
        self._userList = []
    def is_attribute(self, attrName):
        i=0
        while self._userAttributes.__len__() > i:
            if self._userAttributes[i] == attrName:
                return True
            i +=1
        return False
    def get_user(self, key, value):
        if not self.is_attribute(key):
            return False
        i=0
        while self._userList.__len__() > i:
            if self._userList[i].get_data(key) == value:
                return self._userList[i].get_all_data()
            i +=1
        return False
    def del_user(self, name):
        i=0
        while self._userList.__len__() > i:
            if self._userList[i].get_data("name") == name:
                del self._userList[i]
                return  True
            i +=1
        return False
        
    def add_user(self, userData):
        i=self._userAttributes.__len__()-1
        while i > -1:
            if self._userAttributes[i] == "name":
                break
            i-=1
        if not self.get_user("name",userData[i]):
            self._userList.append(User(self._userAttributes, userData))
            return True
        print("Did not add user {}".format(userData[i]))
        return False
    def get_users(self):
        temp = {}
        i=0
        while self._userList.__len__() > i:
            temp[i] = self._userList[i].get_all_data()
            i+=1
        return temp
    def add_attribute(self, attrName, defaultVal):
        if self.is_attribute(attrName):
            return False
        self._userAttributes.append(attrName)
        i=0
        while self._userList.__len__() > i:
            self._userList[i].set_attributes(attrName, defaultVal)
            i+=1
        return True
    def set_attribute(self, user, attrName, value):
        if not self.is_attribute(attrName):
            return False
        tuser = self.get_user("name", user)
        if type(tuser) == dict:
            tuser[attrName] = value
            return True
        return False
    def set_psw(self, username):
        npsw = ""
        i = 0
        while i < 10:
            npsw += lalphabet[random.randint(0, len(lalphabet)-1)]
            i += 1
        m = md5.new()
        m.update(npsw)
        self.set_attribute(username, "password", m.hexdigest())
        return npsw
    def check_psw(self):
        i=0
        while self._userList.__len__() > i:
            p=self._userList[i].get_data("password")
            if p.__len__() != 32:
                return False
            if not all(c in "0123456789abcdef" for c in p):
                return False
            i+=1
        return True
    def make_copy(self):
        tstr = "/" + userFile + str(datetime.datetime.now())[:22]
        tstr = tstr.replace(' ','_')
        tstr = tstr.replace(':','-')
        tstr = tstr.replace('.dat','')
        tstr += ".dat"
        if not os.path.exists(userDirectory+"/archives"):
            os.makedirs(userDirectory+"/archives")
        os.rename(userDirectory + "/" + userFile,userDirectory+ "/archives" + tstr)
        return str(userDirectory+ "/archives" + tstr)
    def write_in_file(self):
        print("User file old copy made in " +self.make_copy())
        f = open(userDirectory + "/" + userFile,"w")
        tstr = ""
        i=0
        while self._userAttributes.__len__() > i:
            tstr += self._userAttributes[i] + " "
            i+=1
        tstr +="\n"
        f.write(tstr)
        i=0
        while self._userList.__len__() > i:
            j=0
            tstr = ""
            while self._userAttributes.__len__() > j:
                tstr += str(self._userList[i].get_data(self._userAttributes[j])) + " "
                j += 1
            tstr +="\n"
            f.write(tstr)
            i += 1
        f.close()

# -------------------------------------
def listenSerial():
    while isListening:
        for m in motes.getMotes():

            length = m.tryRead(binaryToo = configInstance.configMode)
            if length == 0:
                continue

            if configInstance.configMode:
                for c in m.buffer:
                    configInstance.byteRead(c)
                m.buffer = ""
                continue

            while '\n' in m.buffer:
                pos = m.buffer.find('\n')
                if pos != 0:
                    newString = m.buffer[:pos].strip()
                    # print "got", newString
                    moteData.addNewData(newString, m.port.portstr)
                m.buffer = m.buffer[pos + 1:]

        moteData.fixSizes()
        # pause for a bit
        time.sleep(0.01)


def closeAllSerial():
    global listenThread
    global isListening
    isListening = False
    if listenThread:
        listenThread.join()
        listenThread = None
    for m in motes.getMotes():
        m.closeSerial()


def openAllSerial():
    global listenThread
    global isListening
    moteData.reset()
    if isListening: return
    isListening = True
    listenThread = threading.Thread(target = listenSerial)
    listenThread.start()
    for m in motes.getMotes():
        m.tryToOpenSerial(False)


def getMansOSVersion():
    path = settingsInstance.getCfgValue("mansosDirectory")
    result = ""
    try:
        with open(os.path.join(path, "doc/VERSION")) as versionFile:
            result = versionFile.readline().strip()
    except Exception as e:
        print(e)
    return result

# --------------------------------------------

class HttpServerHandler(BaseHTTPRequestHandler):
    server_version = 'MansOS/' + getMansOSVersion() + ' Web Server'
    protocol_version = 'HTTP/1.1' # 'HTTP/1.0' is the default, but we want chunked encoding
    def writeChunk(self, buffer):
        if self.wfile == None: return
        if self.wfile._sock == None: return
        self.wfile.write("{:x}\r\n".format(len(buffer)))
        self.wfile.write(buffer)
        self.wfile.write("\r\n")

    def writeFinalChunk(self):
        self.wfile.write("0\r\n")
        self.wfile.write("\r\n")

    # overrides base class function, because in some versions
    # it tries to resolve dns and fails...
    def log_message(self, format, *args):
        sys.stderr.write("%s - - [%s] %s\n" %
                         (self.client_address[0],
                          self.log_date_time_string(),
                          format%args))
        
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
                tlist=self.headers["Cookie"].split(cookieName)
                if value == "":
                    tlist[1]=tlist[1][tlist[1].find(";"):]
                else:
                    tlist[0]+=cookieName+"="
                    tlist[1]=value+tlist[1][tlist[1].find(";"):]
                self.headers["Cookie"]=tlist[0]+tlist[1]
            else:
                if value != "":
                    self.headers["Cookie"]+="; "+cookieName+"="+value
        else:
            if value != "":
                self.headers["Cookie"]=cookieName+"="+value
    def setSafe(self,state):
        self.headers["Safe"]=state
        return
    def isSafe(self):
        if "Safe" in self.headers:
            if self.headers["Safe"] == "True":
                return True
        return False
    
    def setSession(self, qs):
        csma=self.getCookie("Msma37")
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
            tsma = tsma + qs["sma"][0][-1:]
            if "log" in qs:
                if qs["log"] == "in":
                    tsid = random.randint(100000000, 999999999)
                    tuser=allUsers.get_user("name", qs["user"][0])
                    if "level" in tuser:
                        tsma = tsma[:-1]+tuser["level"]
                    else:
                        tsma = tsma[:-1]+"1"
                    if qs["sma"][0][:2].isdigit():
                        fi=int(qs["sma"][0][:2])
                    else:
                        fi=0
                    fi=fi%25
                    tcod=int(tuser["password"][fi:fi+5],16)
                    qs["tsid"]=tsid-tcod
                    tsid=str(tsid)
                    msid = md5.new()
                    msid.update(tsid)
                    tsid=msid.hexdigest()
                    allSessions.add_sid(qs["sma"][0], tsid, tuser)
                    ms = md5.new()
                    ms.update(tsid+qs["sma"][0])
                    tsid = ms.hexdigest()
                    self.changeHeadersCookie("Msid37",tsid)
                    if not "sid" in qs:
                        qs["sid"] = []
                        qs["sid"].append(tsid)
                    else:
                        qs["sid"][0] = tsid
                elif qs["log"] == "out":
                    tsma = tsma[:-1]+"0"
                    qs["del"]="yes"
                    allSessions.del_sid(qs["sma"][0])
                    if "sid" in qs:
                        del qs["sid"]
                    self.changeHeadersCookie("Msid37","")
            if "sid" in qs:
                tses=allSessions.get_session(qs["sma"][0])
                if tses:
                    if hasattr(tses, "_sid"):
                        m = md5.new()
                        m.update(tses._sid+qs["sma"][0])
                        if m.hexdigest() == qs["sid"][0]:
                            self.setSafe("True")
                        else:
                            self.setSafe("False")
                            print("Possible security intrusions attempted!")
                            allSessions.del_session(tses._sma)
            if not allSessions.set_sma(qs["sma"][0],tsma):
                tsma= tsma[:-1]+"0"
                self.changeHeadersCookie("Msid37","")
                if "sid" in qs:
                     del qs["sid"]
                self.setSafe("False")
                qs["del"]="yes"
        else:
            tsma = tsma + "0"
            allSessions.add_session(tsma)
            qs["del"]="yes"
        self.changeHeadersCookie("Msma37",tsma)
        if not "sma" in qs:
            qs["sma"] = []
            qs["sma"].append(tsma)
        else:
            qs["sma"][0] = tsma
        #print("This session is safe {}".format(self.isSafe()))
        #print("allSessions = ")
        #print(allSessions.get_sessions()
            
    def serveSession(self, qs, urlTo):
        with open(htmlDirectory + "/session.html", "r") as f:
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
            
    def getLevel(self, qs={}):
        if "sma" in qs:
            if qs["sma"][0][-1:].isdigit():
                return int(qs["sma"][0][-1:])
        csma=self.getCookie("Msma37")
        if csma:
            if csma[-1:].isdigit():
                return int(csma[-1:])
            return 0
        else:
            return 0
        
    def haveAccess(self, qs={}):
      try:
        if "sma" in qs:
            if qs["sma"][0][-1:] != "0":
                tsu=allSessions.get_session(qs["sma"][0])
                if tsu:
                    tsu=tsu._user
                    if tsu.get("hasWriteAccess",False) == "True":
                        return True
            return False
        csma=self.getCookie("Msma37")
        if csma:
            if csma[-1:] != "0":
                tsu=allSessions.get_session(csma)
                if tsu:
                    tsu=tsu._user
                    if tsu.get("hasWriteAccess",False) == "True":
                        return True
            return False
      except:
        return False

    def serveHeader(self, name, qs = {"no" : "no"}, isGeneric = True, includeBodyStart = True, replaceValues = None, urlTo = ""):
        self.headerIsServed = True
        if name == "default":
            pagetitle = ""
        else:
            pagetitle = " &#8211; " + toTitleCase(name)

        with open(htmlDirectory + "/header.html", "r") as f:
            contents = f.read()
            contents = contents.replace("%PAGETITLE%", pagetitle)
            self.writeChunk(contents)
        try:
            with open(htmlDirectory + "/" + name + ".header.html", "r") as f:
                contents = f.read()
                if replaceValues:
                    for v in replaceValues:
                        contents = contents.replace("%" + v + "%", replaceValues[v])
                self.writeChunk(contents)
        except:
            pass

        if includeBodyStart:
            try:
                if not "no" in qs:
                    self.serveSession(qs, urlTo)
            except Exception as e:
                print("Error Session not served!:")
                print(e)
                self.writeChunk('</head>\n<body>')

            with open(htmlDirectory + "/top-start.html", "r") as f:
                contents = f.read()
                if replaceValues:
                    for v in replaceValues:
                        contents = contents.replace("%" + v + "%", replaceValues[v])
                # page title
                contents = contents.replace("%PAGETITLE%", pagetitle)
                # this page (for form)
                contents = contents.replace("%THISPAGE%", name)
                self.writeChunk(contents)
            
            suffix = "generic" if isGeneric else "mote"
            with open(htmlDirectory + "/menu-" + suffix + ".html", "r") as f:
                contents = f.read()
                if replaceValues:
                    for v in replaceValues:
                        contents = contents.replace("%" + v + "%", replaceValues[v])
                if "sma" in qs: contents = contents.replace("%SMA%", qs["sma"][0])
                self.writeChunk(contents)
            if isGeneric:
                if self.getLevel() > 0:
                    with open(htmlDirectory + "/menu-1.html", "r") as f:
                        contents = f.read()
                        if replaceValues:
                            for v in replaceValues:
                                contents = contents.replace("%" + v + "%", replaceValues[v])
                        if "sma" in qs: contents = contents.replace("%SMA%", qs["sma"][0])
                        self.writeChunk(contents)
                if self.getLevel() > 7:
                    with open(htmlDirectory + "/menu-8.html", "r") as f:
                        contents = f.read()
                        if replaceValues:
                            for v in replaceValues:
                                contents = contents.replace("%" + v + "%", replaceValues[v])
                        if "sma" in qs: contents = contents.replace("%SMA%", qs["sma"][0])
                        self.writeChunk(contents)

            with open(htmlDirectory + "/top-end.html", "r") as f:
                contents = f.read()
                if replaceValues:
                    for v in replaceValues:
                        contents = contents.replace("%" + v + "%", replaceValues[v])
                if "sma" in qs: contents = contents.replace("%SMA%", qs["sma"][0])
                # login/logout
                log = "Logout" if self.getLevel() > 0 else "Login"
                contents = contents.replace("%LOG%", log)
                self.writeChunk(contents)

    def serveBody(self, name, qs = {'sma': ['0000000'],}, replaceValues = None):
        disabled = "" if self.haveAccess(qs) else 'disabled="disabled" '
        with open(htmlDirectory + "/" + name + ".html", "r") as f:
            contents = f.read()
            if replaceValues:
                for v in replaceValues:
                    contents = contents.replace("%" + v + "%", replaceValues[v])
            contents = contents.replace("%DISABLED%", disabled)
            if "sma" in qs: contents = contents.replace("%SMA%", qs["sma"][0])
            self.writeChunk(contents)


    def serveMotes(self, action, namedAction, qs, isPost):
        text = ''
        if isPost:
            text += '<form method="post" enctype="multipart/form-data" action="' + action + '">'
        else:
            text += '<form action="' + action + '">'
        self.writeChunk(text)
        disabled = "" if self.haveAccess(qs) else 'disabled="disabled" '
        c = ""
        i = 0
        for m in motes.getMotes():
            name = "mote" + str(i)

            if name in qs:
                m.isSelected = qs[name][0] == 'on'
            elif "action" in qs:
                m.isSelected = False

            checked = ' checked="checked"' if m.isSelected else ""

            c += '<div class="mote"><strong>Mote: </strong>' + m.portName
            c += ' (<strong>Platform: </strong>' + m.platform + ') '
            c += ' <input type="checkbox" title="Select the mote" name="' + name + '"'
            c += checked + ' ' + disabled + '/>' + namedAction + '</div>\n'
            i += 1

        # remember which motes were selected and which were not
        motes.storeSelected()

        if c:
            c = '<div class="motes1">\nDirectly attached motes:\n<br/>\n' + c + '</div>\n'
            self.writeChunk(c)

        self.writeChunk('<div class="form">\n')


    def serveMoteMotes(self, qs):
        if motes.isEmpty():
            self.serveError("No motes connected!")
            return

        text = '<form action="config"><div class="motes2">\n'
        text += 'Directly attached motes:<br/>\n'
        
        disabled = "" if self.haveAccess(qs) else 'disabled="disabled" '

        i = 0
        for m in motes.getMotes():
            name = "mote" + str(i)
            text += '<div class="mote"><strong>Mote: </strong>' + m.portName
            text += ' <input type="hidden" name="sma" class="Msma37" value="0"> '
            text += ' <input type="submit" name="' + name \
                + '_cfg" title="Get/set mote\'s configuration (e.g. sensor reading periods)" value="Configuration..." ' + disabled + '/>\n'
            text += ' <input type="submit" name="' + name \
                + '_files" title="View files on mote\'s filesystem" value="Files..." ' + disabled + '/>\n'
            text += ' Platform: <select name="sel_' + name \
                + '" ' + disabled + ' title="Select the mote\'s platform: determines the list of sensors the mote has. Also has effect on code compilation and uploading">\n'
            for platform in supportedPlatforms:
                selected = ' selected="selected"' if platform == m.platform else ''
                text += '  <option value="' + platform + '"' + selected + '>' + platform + '</option>\n'
            text += ' </select>\n'
            text += '</div>\n'

            i += 1

        text += '<input type="submit" name="platform_set" value="Update platforms" ' + disabled + '/><br/>\n'
        text += "</div></form>"
        self.writeChunk(text)


    def serveFooter(self):
        with open(htmlDirectory + "/footer.html", "r") as f:
            contents = f.read()
            self.writeChunk(contents)
        self.writeFinalChunk()

    def sendDefaultHeaders(self):
        self.send_header('Content-Type', 'text/html')
        # use chunked transfer encoding (to be able to send additional chunks 'later')
        self.send_header('Transfer-Encoding', 'chunked')
        # disable caching
        self.send_header('Cache-Control', 'no-store');
        self.send_header('Connection', 'close');

    def serveFile(self, filename):
        mimetype = 'text/html'
        if filename[-4:] == '.css': mimetype = 'text/css'
        elif filename[-3:] == '.js': mimetype = 'application/javascript'
        elif filename[-4:] == '.png': mimetype = 'image/png'
        elif filename[-4:] == '.gif': mimetype = 'image/gif'
        elif filename[-4:] == '.jpg': mimetype = 'image/jpg'
        elif filename[-4:] == '.tif': mimetype = 'image/tif'

        try:
            with open(filename, "rb") as f:
                contents = f.read()
                self.send_response(200)
                self.send_header('Content-Type', mimetype)
                self.send_header('Content-Length', str(len(contents)))
                self.end_headers()
                self.wfile.write(contents)
                f.close()
        except:
            print("problem with file " + filename + "\n")
            self.serve404Error(filename, {})

    def serve404Error(self, path, qs):
        #self.setSession(qs)
        self.send_response(404)
        self.sendDefaultHeaders()
        self.end_headers()
        qs["no"] = "no"
        self.serveHeader("404", qs)
        self.writeChunk("<strong>Path " + path + " not found on the server</strong>\n")
        self.serveFooter()

    def serveError(self, message, serveFooter = True):
        if not self.headerIsServed:
            self.serveHeader("error")
        self.writeChunk("\n<h4 class='err'>Error: " + message + "</h4>\n")
        if serveFooter:
            self.serveFooter()

    def serveDefault(self, qs, isSession = False):
        if not isSession:
            self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        if isSession:
            self.serveHeader("default", qs, True, True, None, "default")
        else:
            self.serveHeader("default", qs)
        self.serveBody("default", qs)
        self.serveFooter()

    def serveEditUsers(self, qs):
        tses = allSessions.get_session(qs["sma"][0])
        webAttributes = settingsInstance.getCfgValue("userWebAttributes")
        user = allUsers.get_user(webAttributes[0], tses.from_code(qs["edituser"][0]))
        if not user:
            del qs["edituser"]
            self.serveUsers(qs, True)
            return
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("Edit user", qs)
        changes = {}
        if self.isSafe():
            tabCode = "<form>"
            for atr in webAttributes:
                tcod = str(random.randint(10000000, 99999999))
                if atr == "name":
                    tabCode += "<p> Name: <strong>" + tses.to_code(user.get(atr, "")) + "</strong><input type='hidden' class='coded tocode' id='" + tcod + "' value=\"" + tses.to_code(user.get(atr, ""),False,tcod) + "\" name='name'></p>"
                else:
                    tabCode += "<p>" + atr + ": " + "<input type='text' class='coded tocode' id='" + tcod + "' value=\"" + tses.to_code(user.get(atr, ""),False,tcod) + "\" name='" + atr + "'></p>"
            tabCode += "<p><input type='checkbox' class='md5' id='yesplease' name='password'>Reset passsord.</p>"
            tabCode += "<input type='submit' id='randtextsave' onclick='return userSave()' name='saveuser' value= 'Save'>"
            tabCode += "<input type='submit' id='deleteuser' onclick='return userDelete()' name='delete' value= 'Delete user'>"
            tabCode += "</form>"
            changes["FORM"] = tabCode
        else:
            changes["FORM"] = ''

        self.serveBody("edituser", qs, changes)
        self.serveFooter()

    def serveAddUsers(self, qs, info = ''):
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("Add user", qs)
        changes = {}
        changes["INFO"] = info
        self.serveBody("adduser", qs, changes)
        self.serveFooter()

    def serveUsers(self, qs, isSession = False):
        if not isSession:
            self.setSession(qs)
        if not self.getLevel() > 7:
            self.serveDefault(qs, True)
            return
        tses = allSessions.get_session(qs["sma"][0])
        webAttributes = settingsInstance.getCfgValue("userWebAttributes")
        changes = {}
        changes["INFO"] = ''
        changes["PSW"] = ''
        if self.getLevel() > 8:
            if "delete" in qs:
                #delete users
                if qs["delete"][0] == tses.to_md5("deleteuser") and self.isSafe() and "name" in qs:
                    user = allUsers.get_user("name", tses.from_code(qs["name"][0]))
                    if user:
                        if not user.get("name", "") in tabuList:
                            if allUsers.del_user(tses.from_code(qs["name"][0])):
                                changes["INFO"] = "<h4 class='suc'> Deleted user " + tses.to_code(tses.from_code(qs["name"][0])) + "! </h4>"
                                #allUsers.write_in_file()
                            else:
                                changes["INFO"] = "<h4 class='err'> Could not delete user " + tses.to_code(tses.from_code(qs["name"][0])) + "! </h4>"
                        else:
                            changes["INFO"] = "<h4 class='err'> It is not allowed to delete " + tses.to_code(tses.from_code(qs["name"][0])) + "! </h4>"
                    else:
                        changes["INFO"] = "<h4 class='err'> Could not delete user " + tses.to_code(tses.from_code(qs["name"][0])) + "! </h4>"
                else:
                    changes["INFO"] = "<h4 class='err'> Wrong data for deleting! </h4>"
            elif "saveuser" in qs:
                #save changes
                if qs["saveuser"][0] == tses.to_md5("randtextsave") and self.isSafe() and "name" in qs:
                    user = allUsers.get_user("name", tses.from_code(qs["name"][0]))
                    if user:
                        username = user["name"]
                        for atr in webAttributes:
                            if not atr in qs:
                                continue
                            if atr == "name":
                                continue
                            if atr == "password": #should not be here
                                continue
                            if atr == "level" and username in tabuList:
                                continue
                            allUsers.set_attribute(username, atr, tses.from_code(qs[atr][0]))
                        changes["INFO"] = "<h4 class='suc'> Saved user " + tses.to_code(username) + "! </h4>"
                        #password
                        if "password" in qs:
                            if qs["password"][0] == tses.to_md5("yesplease"):
                                if username in tabuList:
                                    changes["PSW"] = "<h4 class='err'>  Can not reset " + tses.to_code(username) + " password.</h4>"
                                elif "level" in qs:
                                    if tses.from_code(qs["level"][0]) == "9":
                                        changes["PSW"] = "<h4 class='err'>  Can not reset level 9 password.</h4>"
                                    else:
                                        changes["PSW"] = "<h4 class='suc'>  Password is " + tses.to_code(allUsers.set_psw(username)) + ".</h4>"
                                else:
                                    tuser = allUsers.get_user("name", qs["user"][0])
                                    if "level" in tuser and tuser["level"] == "9":
                                        changes["PSW"] = "<h4 class='err'>  Can not reset level 9 password.</h4>"
                                    else:
                                        changes["PSW"] = "<h4 class='suc'>  Password is " + tses.to_code(allUsers.set_psw(username)) + ".</h4>"
                        #allUsers.write_in_file()
                    else:
                        changes["INFO"] = "<h4 class='err'> Could not save user " + tses.to_code(tses.from_code(qs["name"][0])) + "! </h4>"
                else:
                    changes["INFO"] = "<h4 class='err'> Wrong data for saving! </h4>"
            elif "useradd" in qs:
                if "username" in qs:
                    tusername = tses.from_code(qs["username"][0])
                    if allUsers.get_user("name", tusername):
                        self.serveAddUsers(qs,"<h4 class='err'> User with name " + tses.to_code(tusername) + " already exists! </h4>")
                    else:
                        dv = settingsInstance.getCfgValue("defaultValues")
                        ua = settingsInstance.getCfgValue("userAttributes")
                        userdata = []
                        for atr in allUsers._userAttributes:
                            if atr == "name":
                                userdata.append(tusername)
                                continue
                            i = ua.__len__() - 1
                            while i > -1:
                                if atr == ua[i]:
                                    userdata.append(dv[i])
                                    break
                                i -= 1
                            if i == -1:
                                userdata.append("None")
                        if allUsers.add_user(userdata):
                            changes["INFO"] = "<h4 class='suc'> New user " + tses.to_code(tusername) + " added!</h4>"
                            changes["PSW"] = "<h4 class='suc'>  Password is " + tses.to_code(allUsers.set_psw(tusername)) + ".</h4>"
                            #allUsers.write_in_file()
                        else:
                            changes["INFO"] = "<h4 class='err'> Could not add user " + tses.to_code(tusername) + "! </h4>"
                else:
                    self.serveAddUsers(qs)
                    return
            elif "edituser" in qs:
                self.serveEditUsers(qs)
                return
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("users", qs)
        if self.isSafe():
            #user tabula
            tabCode = "<table class='table' id='usertable'><tr>"
            for atr in webAttributes:
                tabCode += "<th>" + tses.to_code(atr) + "</th>"
            tabCode += "</tr>"
            for user in allUsers._userList:
                tabCode += "<tr>"
                for atr in webAttributes:
                    tabCode += "<td>" + tses.to_code(user.get_data(atr)) + "</td>"
                tabCode += "</tr>"
            tabCode += "</table>"
            changes["TAB"] = tabCode
        else:
            changes["TAB"] = ''

        self.serveBody("users", qs, changes)
        self.serveFooter()
        
    def serveAccount(self, qs, isSession = False):
        if not isSession:
            self.setSession(qs)
        if not self.getLevel() > 0:
            self.serveDefault(qs, True)
            return
        changes = {}
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("account", qs)
        self.serveBody("account", qs, changes)
        self.serveFooter()
        
    def serveLogin(self, qs):
        csma=self.getCookie("Msma37")
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
            tuser=allUsers.get_user("name", qs["user"][0])
            if tuser:
                m = md5.new()
                m.update(tuser["password"]+qs["sma"][0])
                if tuser and m.hexdigest() == qs["password"][0]:
                    qs["log"] = "in"
                    self.setSession(qs)
                    self.serveDefault(qs, True)
                    return
            changes["FAIL"]="You have made a mistake!"
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("login", qs)
        self.serveBody("login", qs, changes)
        self.serveFooter()

    def serveMoteSelect(self, qs):
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("motes", qs)
        self.serveMoteMotes(qs)
        self.serveFooter()

    def serveConfig(self, qs):
        self.setSession(qs)
        if not self.getLevel() > 1:
            self.serveDefault(qs, True)
            return
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()

        if "platform_set" in qs:
            i = 0
            for m in motes.getMotes():
                motename = "sel_mote" + str(i)
                if motename in qs:
                    m.platform = qs[motename][0]
                i += 1
            motes.storeSelected()

            text = '<strong>Mote platforms updated!</strong>\n'
            self.serveHeader("config", qs, isGeneric = True)
            self.writeChunk(text)
            self.serveFooter()
            return

        openAllSerial()

        moteIndex = None
        filesRequired = False
        for s in qs:
            if s[:4] == "mote":
                pair = s[4:].split('_')
                try:
                    moteIndex = int(pair[0])
                    filesRequired = pair[1] == "files"
                except:
                    pass
                break

        if moteIndex is None or moteIndex >= len(motes.getMotes()):
            self.serveError("Config page requested, but mote not specified!")
            return

        platform = None
        dropdownName = "sel_mote" + str(moteIndex)
        if dropdownName in qs:
            platform = qs[dropdownName][0]

        if platform not in supportedPlatforms:
            self.serveError("Config page requested, but platform not specified or unknown!")
            return

        moteidQS = "?sel_mote" + str(moteIndex) + "=" + platform + "&" + "mote" + str(moteIndex)
        self.serveHeader("config", qs, isGeneric = False,
                         replaceValues = {"MOTEID_CONFIG" : moteidQS + "_cfg=1",
                          "MOTEID_FILES" : moteidQS + "_files=1"})

        configInstance.setMote(motes.getMote(moteIndex), platform)

        (errmsg, ok) = configInstance.updateConfigValues(qs)
        if not ok:
            self.serveError(errmsg)
            return

        # fill config values from the mote / send new values to the mote
        if "get" in qs:
            reply = configInstance.getConfigValues()
            #self.writeChunk(reply)
        elif "set" in qs:
            reply = configInstance.setConfigValues()
            #self.writeChunk(reply)

        if filesRequired:
            if "filename" in qs:
                (text, ok) = configInstance.getFileContentsHTML(qs)
            else:
                (text, ok) = configInstance.getFileListHTML(moteIndex)
        else:
            (text, ok) = configInstance.getConfigHTML()
        if not ok:
            self.serveError(text)
            return
        self.writeChunk(text)
        self.serveFooter()

    def serveGraphs(self, qs):
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("graph", qs)
        self.serveMotes("graph", "Listen", qs, False)

        if "action" in qs and self.getLevel() > 1:
            if qs["action"][0] == "Start":
                if not motes.anySelected():
                    self.serveError("No motes selected!", False)
                elif isListening:
                    self.serveError("Already listening!", False)
                else:
                    openAllSerial()
            else:
                closeAllSerial()

        action = "Stop" if isListening else "Start"
        self.serveBody("graph", qs, {"MOTE_ACTION": action})
        self.serveFooter()

    def serveListen(self, qs):
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("listen", qs)
        self.serveMotes("listen", "Listen", qs, False)

        if "action" in qs and self.getLevel() > 1:
            if qs["action"][0] == "Start":
                if not motes.anySelected():
                    self.serveError("No motes selected!", False)
                if isListening:
                    self.serveError("Already listening!", False)
                openAllSerial()
            else:
                closeAllSerial()

        txt = ""
        for line in moteData.listenTxt:
            txt += line + "<br/>"

        action = "Stop" if isListening else "Start"

        if "dataFile" in qs:
            dataFilename = qs["dataFile"][0]
            if len(dataFilename) and dataFilename.find(".") == -1:
                dataFilename += ".csv"
        else:
            dataFilename = settingsInstance.getCfgValue("saveToFilename")

        saveMultipleFiles = settingsInstance.getCfgValueAsBool("saveMultipleFiles")
        saveProcessedData = settingsInstance.getCfgValueAsBool("saveProcessedData")
        if "dataType" in qs:
            saveProcessedData = not qs["dataType"][0] == "raw"
            saveMultipleFiles = qs["dataType"][0] == "mprocessed"

        rawdataChecked = not saveProcessedData
        sprocessedChecked = saveProcessedData and not saveMultipleFiles
        mprocessedChecked = saveProcessedData and saveMultipleFiles

        settingsInstance.setCfgValue("saveToFilename", dataFilename)
        settingsInstance.setCfgValue("saveProcessedData", bool(saveProcessedData))
        settingsInstance.setCfgValue("saveMultipleFiles", bool(saveMultipleFiles))
        settingsInstance.save()

        self.serveBody("listen", qs,
                       {"LISTEN_TXT" : txt,
                        "MOTE_ACTION": action,
                        "DATA_FILENAME" : dataFilename,
                        "RAWDATA_CHECKED" : 'checked="checked"' if rawdataChecked else "",
                        "SPROCDATA_CHECKED" : 'checked="checked"' if sprocessedChecked else "",
                        "MPROCDATA_CHECKED" : 'checked="checked"' if mprocessedChecked else ""})
        self.serveFooter()

    def serveUpload(self, qs):
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("upload", qs)
        self.serveMotes("upload", "Upload", qs, True)
        isSealCode = settingsInstance.getCfgValueAsInt("isSealCode")
        isSlow = settingsInstance.getCfgValueAsInt("slowUpload")
        self.serveBody("upload", qs,
                       {"CCODE_CHECKED": 'checked="checked"' if not isSealCode else "",
                        "SEALCODE_CHECKED" : 'checked="checked"' if isSealCode else "",
                        "UPLOAD_CODE" : lastUploadCode,
                        "UPLOAD_CONFIG" : lastUploadConfig,
                        "UPLOAD_FILENAME": lastUploadFile,
                        "SLOW_CHECKED" : 'checked="checked"' if isSlow else ""})
        self.serveFooter()

    def uploadCallback(self, line):
        global uploadResult
        uploadResult += line
        return True

    def serveUploadResult(self, qs):
        global uploadResult
        global isInSubprocess
        isInSubprocess = True
        try:
            self.setSession(qs)
            self.send_response(200)
            self.sendDefaultHeaders()
            self.end_headers()
            self.serveHeader("upload", qs)
            self.writeChunk('<button type="button" onclick="window.open(\'\', \'_self\', \'\'); window.close();">OK</button><br/>')
            self.writeChunk("Upload result:<br/><pre>\n")
#??
            while isInSubprocess or uploadResult:
                if uploadResult:
                    self.writeChunk(uploadResult)
                    uploadResult = ""
                else:
                    time.sleep(0.001)
            self.writeChunk("</pre>\n")
            self.serveFooter()
        except:
            raise
        finally:
            uploadResult = ""

    def serveBlockly(self, qs):
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("blockly", qs)
        self.serveBody("blockly", qs)
        self.serveFooter()

    def serveSealFrame(self, qs):
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        path = os.path.join(sealBlocklyPath, "index.html")
        with open(path) as f:
            contents = f.read()
            disabled = 'disabled="disabled"' if not self.haveAccess() else ""
            contents = contents.replace("%DISABLED%", disabled)
            self.writeChunk(contents)
        self.writeFinalChunk()

    # Dummy, have to respond somehow, so javascript knows we are here
    def serveSync(self, qs):
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        if self.haveAccess(qs):
            self.writeChunk("writeAccess=True")
        self.writeFinalChunk()

    def serveGraphsData(self, qs):
        global lastJsonData

        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()

        # if not listening at the moment,
        # but were listening previosly, send the previous data
        if not isListening and lastJsonData :
            self.writeChunk(lastJsonData)
            self.writeFinalChunk()
            return

        # get the data to display in graphs
        if moteData.hasData():
            jsonData = json.JSONEncoder().encode(moteData.getData())
        else:
            jsonData = ""

        lastJsonData = jsonData
        self.writeChunk(jsonData)
        self.writeFinalChunk()

    def serveListenData(self, qs):
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        text = ""
        for line in moteData.listenTxt:
            text += line + "<br/>"
        if text:
            self.writeChunk(text)
        self.writeFinalChunk()

    def do_GET(self):
        self.headerIsServed = False

        o = urlparse(self.path)
        qs = parse_qs(o.query)

        if o.path == "/" or o.path == "/default":
            self.serveDefault(qs)
        elif o.path == "/motes":
            self.serveMoteSelect(qs)
        elif o.path == "/config":
            self.serveConfig(qs)
        elif o.path == "/graph":
            self.serveGraphs(qs)
        elif o.path == "/graph-data":
            self.serveGraphsData(qs)
        elif o.path == "/upload":
            self.serveUpload(qs)
        elif o.path == "/login":
            self.serveLogin(qs)
        elif o.path == "/account":
            self.serveAccount(qs)
        elif o.path == "/users":
            self.serveUsers(qs)
        elif o.path == "/upload-result":
            self.serveUploadResult(qs)
        elif o.path == "/listen":
            self.serveListen(qs)
        elif o.path == "/listen-data":
            self.serveListenData(qs)
        elif o.path == "/blockly":
            self.serveBlockly(qs)
        elif o.path == "/seal-frame":
            self.serveSealFrame(qs)
        elif o.path[:13] == "/seal-blockly":
            self.serveFile(os.path.join(sealBlocklyPath, o.path[14:]))
        elif o.path == "/sync":
            self.serveSync(qs)
        elif o.path == "/code":
            # qs['src'] contains SEAL-Blockly code
            code = qs.get('src')[0] if "src" in qs else ""
            config = qs.get('config')[0] if "config" in qs else ""
            if motes.anySelected():
                self.compileAndUpload(code, config, None, True)
            self.serveSync(qs)
        elif o.path[-4:] == ".css":
            self.serveFile(htmlDirectory + "/css/" + o.path)
        elif o.path[-4:] in [".png", ".jpg", ".gif", ".tif"]:
            self.serveFile(htmlDirectory + "/img/" + o.path)
        elif o.path[-3:] in [".js"]:
            self.serveFile(htmlDirectory + "/js/" + o.path)
        else:
            self.serve404Error(o.path, qs)

    def compileAndUpload(self, code, config, fileContents, isSEAL):
        global lastUploadCode
        global lastUploadConfig
        global lastUploadFile

        retcode = 0
        if not os.path.exists("build"):
            os.mkdir("build")
        # TODO FIXME: should do this in a new thread!
        isInSubprocess = True
        os.chdir("build")

        if fileContents:
            lastUploadFile = form["file"].filename

            filename = "tmp-file.ihex"
            with open(filename, "w") as outFile:
                outFile.write(fileContents)
                outFile.close()

            closeAllSerial()
            for m in motes.getMotes():
                r = m.tryToUpload(self, filename)
                if r != 0: retcode = r

        elif code:
            lastUploadCode = code

            filename = "main."
            filename += "sl" if isSEAL else "c"
            with open(filename, "w") as outFile:
                outFile.write(code)
                outFile.close()

            with open("config", "w") as outFile:
                if config is None:
                    config = ""
                outFile.write(config)
                outFile.close()

            with open("Makefile", "w") as outFile:
                if isSEAL:
                    outFile.write("SEAL_SOURCES = main.sl\n")
                else:
                    outFile.write("SOURCES = main.c\n")
                outFile.write("APPMOD = App\n")
                outFile.write("PROJDIR = $(CURDIR)\n")
                outFile.write("ifndef MOSROOT\n")
                mansosPath = settingsInstance.getCfgValue("mansosDirectory")
                if not os.path.isabs(mansosPath):
                    # one level up - because we are in build directory
                    mansosPath = os.path.join(mansosPath, "..")
                outFile.write("  MOSROOT = " + mansosPath + "\n")
                outFile.write("endif\n")
                outFile.write("include ${MOSROOT}/mos/make/Makefile\n")
                outFile.close()

            closeAllSerial()
            for m in motes.getMotes():
                r = m.tryToCompileAndUpload(self, filename)
                if r != 0 and m.port:
                    retcode = r

        os.chdir("..")
        isInSubprocess = False
        return retcode

    def do_POST(self):
        global lastUploadCode
        global lastUploadConfig
        global lastUploadFile
        global isInSubprocess
        self.headerIsServed = False

        # Parse the form data posted
        form = cgi.FieldStorage(
            fp = self.rfile,
            headers = self.headers,
            environ = {'REQUEST_METHOD':'POST',
                     'CONTENT_TYPE':self.headers['Content-Type'],
                     })
        #self.setSession(qs) ?
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.send_header('Transfer-Encoding', 'chunked')
        self.end_headers()

        file_data = None

        isSEAL = False
        if "compile" in form:
            if "language" in form:
                isSEAL = form["language"].value.strip() == "SEAL"
            settingsInstance.setCfgValue("isSealCode", isSEAL)

        if "slow" in form:
            slow = form["slow"].value == "on"
        else:
            slow = False
        settingsInstance.setCfgValue("slowUpload", slow)
        settingsInstance.save()

        if "code" in form.keys():
            code = form["code"].value
        else:
            code = None

        if "file" in form.keys():
            fileContents = form["file"].file.read()
        else:
            fileContents = None

        # check if what to upload is provided
        if not fileContents and not code:
            self.serveHeader("upload")
            self.serveError("Neither filename nor code specified!")
            return

        i = 0
        for m in motes.getMotes():
            name = "mote" + str(i)
            if name in form:
                isChecked = form[name].value == "on"
            else:
                isChecked = False

            if isChecked:
                m.isSelected = True
            else:
                m.isSelected = False
            i += 1

        # remember which motes were selected and which not
        motes.storeSelected()
        # check if any motes are selected
        if not motes.anySelected():
            self.serveHeader("upload")
            self.serveError("No motes selected!")
            return

        config = ""
        if "config" in form.keys():
            lastUploadConfig = form["config"].value
            config = lastUploadConfig
        if slow:
            config += "\nSLOW_UPLOAD=y\n"

        retcode = self.compileAndUpload(code, config, fileContents, isSEAL)

        self.serveHeader("upload")
        self.serveMotes("upload", "Upload", True)
        if retcode == 0:
            self.writeChunk("<strong>Upload done!</strong></div>")
        else:
            self.writeChunk("<strong>Upload failed!</strong></div>")
        self.serveFooter()
        motes.closeAll()


class ThreadingHTTPServer(ThreadingMixIn, HTTPServer):
    # Overrides BaseServer function to get better control over interrupts
    def serve_forever(self, poll_interval = 0.5):
        """Handle one request at a time until shutdown.

        Polls for shutdown every poll_interval seconds. Ignores
        self.timeout. If you need to do periodic tasks, do them in
        another thread.
        """
        self._BaseServer__is_shut_down.clear()
        try:
            while not self._BaseServer__shutdown_request:
                # XXX: Consider using another file descriptor or
                # connecting to the socket to wake this up instead of
                # polling. Polling reduces our responsiveness to a
                # shutdown request and wastes cpu at all other times.
                r, w, e = select.select([self], [], [], poll_interval)
                if self in r:
                    self._handle_request_noblock()
        finally:
            self._BaseServer__shutdown_request = False
            self._BaseServer__is_shut_down.set()
            if os.name == "posix":
                # kill the process to make sure it exits
                os.kill(os.getpid(), signal.SIGKILL)

# --------------------------------------------
def makeDefaultUserFile():
    if not os.path.exists(userDirectory):
        os.makedirs(userDirectory)
    if not os.path.exists(userDirectory + "/" + userFile):
        uf = open(userDirectory + "/" + userFile,"w")
        for at in settingsInstance.getCfgValue("userAttributes"):
            uf.write(at+" ")
        uf.write("\n")
        for ad in settingsInstance.getCfgValue("adminValues"):
            uf.write(ad+" ")
        uf.write("\n")
        uf.close()
        return str(userDirectory + "/" + userFile)
def initalizeUsers():
    global allUsers
    global allSessions
    global userFile
    global userDirectory
    
    allSessions = Sessions()
    
    userDirectory = os.path.abspath(settingsInstance.getCfgValue("userDirectory"))
    userFile = settingsInstance.getCfgValue("userFile")
    if not os.path.exists(userDirectory + "/" + userFile):
        print("No user file. Python add default in "+makeDefaultUserFile())
        
    uf = open(userDirectory + "/" + userFile,"r")
    i = False
    for line in uf:
         if not i:
             i = True
             allUsers = Users(line.split())
         else:
             allUsers.add_user(line.split())
    uf.close()
    
    if not "name" in allUsers._userAttributes:
        print("User attribute \"name\" required! Python save old user file in "+allUsers.make_copy())
        print("New default file made in "+makeDefaultUserFile())
        uf = open(userDirectory + "/" + userFile,"r")
        i = False
        for line in uf:
             if not i:
                 i = True
                 allUsers = Users(line.split())
             else:
                 allUsers.add_user(line.split())
        uf.close()
    elif not allUsers.get_user("name", "admin"):
        print("No admin! Python save old user file in "+allUsers.make_copy())
        print("New default file made in "+makeDefaultUserFile())
        uf = open(userDirectory + "/" + userFile,"r")
        i = False
        for line in uf:
             if not i:
                 i = True
                 allUsers = Users(line.split())
             else:
                 allUsers.add_user(line.split())
        uf.close()
    elif not "password" in allUsers._userAttributes:
        print("User attribute \"password\" required! Python save old user file in "+allUsers.make_copy())
        print("New default file made in "+makeDefaultUserFile())
        uf = open(userDirectory + "/" + userFile,"r")
        i = False
        for line in uf:
             if not i:
                 i = True
                 allUsers = Users(line.split())
             else:
                 allUsers.add_user(line.split())
        uf.close()
    elif not allUsers.check_psw():
        print("Passwords do not match the md5 standard! Python save old user file in "+allUsers.make_copy())
        print("New default file made in "+makeDefaultUserFile())
        uf = open(userDirectory + "/" + userFile,"r")
        i = False
        for line in uf:
             if not i:
                 i = True
                 allUsers = Users(line.split())
             else:
                 allUsers.add_user(line.split())
        uf.close()


    if not allUsers.get_user("name", "admin") or not "name" in allUsers._userAttributes or not "password" in allUsers._userAttributes or not allUsers.check_psw():
        print("There is something wrong with user.cfg")
    
    ua = settingsInstance.getCfgValue("userAttributes")
    na = set(ua) - set(allUsers._userAttributes)
    if len(na) > 0:
        dv = settingsInstance.getCfgValue("defaultValues")
        av = settingsInstance.getCfgValue("adminValues")
        while len(na) > 0:
            n = na.pop()
            print("New attribute for users: " + str(n))
            i = ua.__len__() - 1
            while i > -1:
                if n == ua[i]:
                    allUsers.add_attribute(ua[i], dv[i])
                    allUsers.set_attribute("admin", ua[i], av[i])
                    break
                i -= 1
        allUsers.write_in_file()
    
def initalizeConfig():
    global htmlDirectory
    global dataDirectory
    global sealBlocklyPath
    
    htmlDirectory = os.path.abspath(settingsInstance.getCfgValue("htmlDirectory"))
    dataDirectory = os.path.abspath(settingsInstance.getCfgValue("dataDirectory"))
    if not os.path.exists(dataDirectory):
        os.makedirs(dataDirectory)
    sealBlocklyPath = os.path.abspath(settingsInstance.getCfgValue("sealBlocklyDirectory"))
    
    initalizeUsers()
        
def main():
    try:
        if settingsInstance.getCfgValueAsBool("createDaemon"):
            # detach from the tty and go to background
            createDaemon()
        initalizeConfig()
        port = settingsInstance.getCfgValueAsInt("port", HTTP_SERVER_PORT)
        server = ThreadingHTTPServer(('', port), HttpServerHandler)
        motes.addAll()
        time.sleep(1)
        print("<http-server>: started, listening to TCP port {}, serial baudrate {}".format(port,
              settingsInstance.getCfgValueAsInt("baudrate", SERIAL_BAUDRATE)))
        server.serve_forever()
    except Exception as e:
        print("<http-server>: exception occurred:")
        print(e)
        return 1

if __name__ == '__main__':
    main()
