#!/usr/bin/env python

#
# MansOS web server - the main file
#

from __future__ import print_function
import os, sys, platform, select, signal, traceback
from string import Template
# add library directory to the path
sys.path.append(os.path.join(os.getcwd(), '..', "lib"))
import pages.page_login as page_login
import pages.page_server as page_server
import pages.page_account as page_account
import pages.page_user as page_user
import pages.page_graph as page_graph
import pages.page_config as page_config
import pages.page_upload as page_upload
import data_utils
import utils
import user
import session
from motes import motes
import moteconfig
import sensor_data
import daemon
import configuration
import mansos_version
import helper_tools as ht

DEBUG = 1

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

lastUploadCode = ""
lastUploadConfig = ""
lastUploadFile = ""
lastData = ""

allSessions = None
allUsers = None

# --------------------------------------------
class HttpServerHandler(BaseHTTPRequestHandler,
                        page_user.PageUser,
                        page_account.PageAccount,
                        page_login.PageLogin,
                        page_server.PageServer,
                        page_graph.PageGraph,
                        page_config.PageConfig,
                        page_upload.PageUpload,
                        session.SetAndServeSessionAndHeader):
    server_version = 'MansOS/' + mansos_version.getMansosVersion(
        configuration.c.getCfgValue("mansosDirectory")) + ' Web Server'
    protocol_version = 'HTTP/1.1' # 'HTTP/1.0' is the default, but we want chunked encoding

    def __init__(self, request, client_address, server):
        #global
        self.sessions = allSessions
        self.users = allUsers
        self.settings = configuration.c
        self.tabuList = session.tabuList
        self.moteData = sensor_data.moteData
        #global end

        self.htmlDirectory = os.path.abspath(self.settings.getCfgValue("htmlDirectory"))
        self.dataDirectory = os.path.abspath(self.settings.getCfgValue("dataDirectory"))
        if not os.path.exists(self.dataDirectory):
            os.makedirs(self.dataDirectory)
        self.sealBlocklyDirectory = os.path.abspath(self.settings.getCfgValue("sealBlocklyDirectory"))

        BaseHTTPRequestHandler.__init__(self, request, client_address, server)

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
                          format % args))

    def sendDefaultHeaders(self):
        self.send_header('Content-Type', 'text/html')
        # use chunked transfer encoding (to be able to send additional chunks 'later')
        self.send_header('Transfer-Encoding', 'chunked')
        # disable caching
        self.send_header('Cache-Control', 'no-store');
        self.send_header('Connection', 'close');



#    def serveHeader(self, name, qs = {"no" : "no"}, isGeneric = True, includeBodyStart = True, replaceValues = None, urlTo = ""):
    def serveHeaderOld(self, name, qs, isGeneric = True, replaceValues = None, urlTo = ""):
        self.headerIsServed = True
        if name == "default":
            pagetitle = ""
        else:
            pagetitle = " &#8211; " + utils.toTitleCase(name)

        with open(self.htmlDirectory + "/header.html", "r") as f:
            contents = f.read()
            contents = contents.replace("%PAGETITLE%", pagetitle)
            self.writeChunk(contents)
        try:
            with open(self.htmlDirectory + "/" + name + ".header.html", "r") as f:
                contents = f.read()
                if replaceValues:
                    for v in replaceValues:
                        contents = contents.replace("%" + v + "%", replaceValues[v])
                self.writeChunk(contents)
        except:
            pass

        try:
            if not "no" in qs:
                text = self.serveSession(qs, urlTo)
                self.writeChunk(text)
        except Exception as e:
            print("Error: Session not served!")
            print(e)
            self.writeChunk('</head>\n<body>')

        with open(self.htmlDirectory + "/top-start.html", "r") as f:
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
        with open(self.htmlDirectory + "/menu-" + suffix + ".html", "r") as f:
            contents = f.read()
            if replaceValues:
                for v in replaceValues:
                    contents = contents.replace("%" + v + "%", replaceValues[v])
            if "sma" in qs: contents = contents.replace("%SMA%", qs["sma"][0])
            self.writeChunk(contents)

        if isGeneric:
            if self.getLevel() > 0:
                with open(self.htmlDirectory + "/menu-1.html", "r") as f:
                    contents = f.read()
                    if replaceValues:
                        for v in replaceValues:
                            contents = contents.replace("%" + v + "%", replaceValues[v])
                    if "sma" in qs: contents = contents.replace("%SMA%", qs["sma"][0])
                    self.writeChunk(contents)
            if self.getLevel() > 7:
                with open(self.htmlDirectory + "/menu-8.html", "r") as f:
                    contents = f.read()
                    if replaceValues:
                        for v in replaceValues:
                            contents = contents.replace("%" + v + "%", replaceValues[v])
                    if "sma" in qs: contents = contents.replace("%SMA%", qs["sma"][0])
                    self.writeChunk(contents)
            if self.getLevel() > 8:
               with open(self.htmlDirectory + "/menu-9.html", "r") as f:
                   contents = f.read()
                   if replaceValues:
                       for v in replaceValues:
                           contents = contents.replace("%" + v + "%", replaceValues[v])
                   if "sma" in qs: contents = contents.replace("%SMA%", qs["sma"][0])
                   self.writeChunk(contents)

        with open(self.htmlDirectory + "/top-end.html", "r") as f:
            contents = f.read()
            if replaceValues:
                for v in replaceValues:
                    contents = contents.replace("%" + v + "%", replaceValues[v])
            if "sma" in qs: contents = contents.replace("%SMA%", qs["sma"][0])
            # login/logout
            log = "Logout" if self.getLevel() > 0 else "Login"
            contents = contents.replace("%LOG%", log)
            self.writeChunk(contents)

    '''
    #########################################################################################
    '''
    
    def serveAnyPage(self, name, qs, isGeneric = True, replaceValues = None, urlTo = "", 
            title = None, content = None, infoMsg = None, errorMsg = None):
        f = open(self.htmlDirectory + "/layout.html", "r")
        contents = f.read()
        t = Template(contents)
        
        self.headerIsServed = True
        if name == "default":
            pageTitle = ""
        elif title != None:
            pageTitle = title
        else:
            pageTitle = " &#8211; " + utils.toTitleCase(name)
           
        if errorMsg != None:
           qs["no"] = "no"   
           
        suffix = "generic" if isGeneric else "mote"
        menuContent = ""
        with open(self.htmlDirectory + "/menu-" + suffix + ".html", "r") as f:
            localMenuContent = f.read()
            if replaceValues:
                for v in replaceValues:
                    localMenuContent = localMenuContent.replace("%" + v + "%", replaceValues[v])
            if "sma" in qs: localMenuContent = localMenuContent.replace("%SMA%", qs["sma"][0])
            menuContent += localMenuContent
            
        if isGeneric:
            if self.getLevel() > 0:
                with open(self.htmlDirectory + "/menu-1.html", "r") as f:
                    localMenuContent = f.read()
                    if replaceValues:
                        for v in replaceValues:
                            localMenuContent = localMenuContent.replace("%" + v + "%", replaceValues[v])
                    if "sma" in qs: localMenuContent = localMenuContent.replace("%SMA%", qs["sma"][0])
                    menuContent += localMenuContent
            if self.getLevel() > 7:
                with open(self.htmlDirectory + "/menu-8.html", "r") as f:
                    localMenuContent = f.read()
                    if replaceValues:
                        for v in replaceValues:
                            localMenuContent = localMenuContent.replace("%" + v + "%", replaceValues[v])
                    if "sma" in qs: localMenuContent = localMenuContent.replace("%SMA%", qs["sma"][0])
                    menuContent += localMenuContent
            if self.getLevel() > 8:
                with open(self.htmlDirectory + "/menu-9.html", "r") as f:
                    localMenuContent = f.read()
                    if replaceValues:
                        for v in replaceValues:
                            localMenuContent = localMenuContent.replace("%" + v + "%", replaceValues[v])
                    if "sma" in qs: localMenuContent = localMenuContent.replace("%SMA%", qs["sma"][0])
                    menuContent += localMenuContent            

        sma = ""
        if "sma" in qs: 
            sma = qs["sma"][0]
        log = "Logout" if self.getLevel() > 0 else "Login"
           
        if name == "error":
            bodyContent = errorMsg
            contents = t.substitute(
                pageTitle = pageTitle, pageHead = "", sessionHead = "", 
                sma = sma, menuContent = menuContent, bodyContent = bodyContent, log = log)
            self.writeChunk(contents)
        elif name == "error:critical":
            bodyContent = "\n<h4 class='err'>Error: " + errorMsg + "</h4>\n"
            contents = t.substitute(
                pageTitle = pageTitle, pageHead = "", sessionHead = "", 
                sma = sma, menuContent = menuContent, bodyContent = bodyContent, log = log)
            self.writeChunk(contents)
        else:   
            pageHead = "" 
            try:
                with open(self.htmlDirectory + "/" + name + ".header.html", "r") as f:
                    pageHead = f.read()
                    if replaceValues:
                        for v in replaceValues:
                            pageHead = pageHead.replace("%" + v + "%", replaceValues[v])
            except:
                pass
            
            hasError = False        
            try:
                if not "no" in qs:
                    sessionHead = self.serveSession(qs, urlTo)
            except Exception as e:
                bodyContent = "Error: Session not served - " + str(e)
                hasError = True            
                            
            if not hasError:
                if content != None:
                    bodyContent = content
                else:    
                    bodyContent = self.serveBody(name, qs, replaceValues)     
        
            sma = ""
            if "sma" in qs: 
                sma = qs["sma"][0]
            log = "Logout" if self.getLevel() > 0 else "Login"
            
            if infoMsg != None:
                bodyContent = infoMsg + bodyContent
            contents = t.substitute(
                pageTitle = pageTitle, pageHead = pageHead, sessionHead = sessionHead, 
                sma = sma, menuContent = menuContent, bodyContent = bodyContent, log = log)
            contents = contents.replace("&#44;", ",")
            self.writeChunk(contents)
    
    def serveDefault(self, qs, isSession = False):
        if not isSession:
            self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        if isSession:
            self.serveAnyPage("default", qs, urlTo = "default")
        else:
            self.serveAnyPage("default", qs)
        
    def serveMoteSelect(self, qs):
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()

        if motes.isEmpty():
            text = "No motes connected!"
            self.serveAnyPage("error:critical", qs, errorMsg = text)
            return

        disabled = "" if self.getLevel() > 1 else 'disabled="disabled" '
        desc = '<div class="mote"><strong>Mote: </strong>${portName}' \
             + '<input type="submit" name="${name}_cfg" ' \
             + 'title="Get/set mote\'s configuration (e.g. sensor reading periods)" ' \
             + 'value="Configuration..." ' + disabled + '/>\n' \
             + '<input type="submit" name="${name}_files" ' \
             + 'title="View files on mote\'s filesystem" value="Files..." ' + disabled + '/>\n' \
             + ' Platform: <select name="sel_${name}"' + disabled + ' ' \
             + 'title="Select the mote\'s platform: determines the list of sensors the mote has. ' \
             + '"Also has effect on code compilation and uploading">\n' \
             + '${details}</select>\n</div>\n'
        detail = '<option value="${platform}" ${selected}>${platform}</option>\n'
            
        text = '<form action="config"><div class="motes2">\n'
        text += 'Directly attached motes:<br/>\n'
        t1 = Template(desc)
        t2 = Template(detail)
        for m in motes.getMotes():
            name = "mote" + m.getPortBasename()
            details = ""
            for platform in moteconfig.supportedPlatforms:
                selected = 'selected="selected"' if platform == m.platform else ''
                details += t2.substitute(platform = platform, selected = selected)
            text += t1.substitute(
                portName = m.getPortName(), name = name, details = details)
        text += '<input type="submit" name="platform_set" value="Update platforms" ' + disabled + '/><br/>\n'
        text += "</div></form>" 
        self.serveAnyPage("motes", qs, content = text)
  
    def serveListen(self, qs):
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()

        motesText = self.serveMotes("listen", "Listen", qs, None)
        errorMsg = None
        global isListening
        if "action" in qs and self.getLevel() > 1:
            if qs["action"][0] == "Start":
                if not motes.anySelected():
                    errorMsg = "\n<h4 class='err'>Error: No motes selected!</h4>\n"
                if isListening:
                    errorMsg = "\n<h4 class='err'>Already listening!</h4>\n"
                else:
                    sensor_data.moteData.reset()
                    ht.openAllSerial()
                    isListening = True
                # Open DB connection
                data_utils.openDBConnection()
            else:
                ht.closeAllSerial()
                isListening = False
                # Close DB connection 
                data_utils.closeDBConnection()

        txt = ""
        for line in sensor_data.moteData.listenTxt:
            txt += line + "<br/>"

        action = "Stop" if isListening else "Start"
        
        dataFilename = configuration.c.getCfgValue("saveToFilename")
        saveProcessedData = configuration.c.getCfgValueAsBool("saveProcessedData")
        
        if self.getLevel() > 1:
            if "dataFile" in qs:
                dataFilename = qs["dataFile"][0]
                if len(dataFilename) and dataFilename.find(".") == -1:
                    dataFilename += ".csv"
            if "dataType" in qs:
                saveProcessedData = not qs["dataType"][0] == "raw"
                saveMultipleFiles = qs["dataType"][0] == "mprocessed"

            configuration.c.setCfgValue("saveToFilename", dataFilename)
            configuration.c.setCfgValue("saveProcessedData", bool(saveProcessedData))
            configuration.c.save()
        
        rawdataChecked = not saveProcessedData
        mprocessedChecked = saveProcessedData

        div_height = "0"
        if "div_height" in qs:
            div_height = qs["div_height"][0]

        self.serveAnyPage("listen", qs, True, {"MOTES_TXT" : motesText,
                        "LISTEN_TXT" : txt,
                        "MOTE_ACTION": action,
                        "DATA_FILENAME" : dataFilename,
                        "RAWDATA_CHECKED" : 'checked="checked"' if rawdataChecked else "",
                        "MPROCDATA_CHECKED" : 'checked="checked"' if mprocessedChecked else "",
                        "DIV_HEIGHT" : div_height}, errorMsg = errorMsg)
        
    def serveMotes(self, action, namedAction, qs, form):
        disabled = "" if self.getLevel() > 1 else 'disabled="disabled" '
        c = ""
        for m in motes.getMotes():
            name = "mote" + m.getPortBasename()

            if qs:
                if name in qs:
                    m.isSelected = qs[name][0] == 'on'
                elif "action" in qs:
                    m.isSelected = False
            elif form:
                if name in form:
                    m.isSelected = form[name].value == "on"
                else:
                    m.isSelected = False

            checked = ' checked="checked"' if m.isSelected else ""

            c += '<div class="mote"><strong>Mote: </strong>' + m.getPortName()
            c += ' (<strong>Platform: </strong>' + m.platform + ') '
            c += ' <input type="checkbox" title="Select the mote" name="' + name + '"'
            c += checked + ' ' + disabled + '/>' + namedAction + '</div>\n'

        # remember which motes were selected and which were not
        motes.storeSelected()

        if c:
            c = '<div class="motes1">\nDirectly attached motes:\n<br/>\n' + c + '</div>\n'

        return c
   
    def serveBlockly(self, qs):
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveAnyPage("blockly", qs)   
   
    def serveSealFrame(self, qs):
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        path = os.path.join(self.sealBlocklyDirectory, "index.html")
        with open(path) as f:
            contents = f.read()
            disabled = 'disabled="disabled"' if not self.getLevel() > 1 else ""
            contents = contents.replace("%DISABLED%", disabled)
            self.writeChunk(contents)
        self.writeFinalChunk()
   
    def serve404Error(self, path, qs):
        self.setSession(qs)
        self.send_response(404)
        self.sendDefaultHeaders()
        self.end_headers()
        qs["no"] = "no"
        self.serveAnyPage("error", qs, 
            errorMsg = "<strong>Error 404: path " + path + " not found on the server</strong>\n")        

    def serveFile(self, filename, qs):
        mimetype = 'text/html'
        if filename[-4:] == '.css':
            mimetype = 'text/css'
            if filename[-9:] == 'theme.css':
                tpath = filename[:-4]
                filename = filename[:-4] + configuration.c.getCfgValue("serverTheme") + '.css'
                theme = self.getCookie("Msma37")
                if theme:
                    theme = allSessions.get_session_old(theme)
                    if theme and hasattr(theme, "_user") and "theme" in theme._user and theme._user["theme"] != "server":
                        # "server" means as same as server
                        theme = tpath + theme._user["theme"] + '.css'
                        if os.path.exists(theme):
                            filename = theme
        elif filename[-3:] == '.js': mimetype = 'application/javascript'
        elif filename[-4:] == '.png': mimetype = 'image/png'
        elif filename[-4:] == '.gif': mimetype = 'image/gif'
        elif filename[-4:] == '.jpg': mimetype = 'image/jpg'
        elif filename[-4:] == '.tif': mimetype = 'image/tif'

        try:
            f = open(filename, "rb")
            contents = f.read()
            self.send_response(200)
            self.send_header('Content-Type', mimetype)
            self.send_header('Content-Length', str(len(contents)))
            self.send_header('Cache-Control', 'public,max-age=1000')
            if DEBUG:
                # enable cache
                self.send_header('Last-Modified', 'Wed, 15 Sep 2004 12:00:00 GMT')
                self.send_header('Expires', 'Sun, 17 Jan 2038 19:14:07 GMT')
            self.end_headers()
            self.wfile.write(contents)
            f.close()
        except:
            print("problem with file " + filename + "\n")
            self.serve404Error(filename, qs)

    def do_GET(self):
        self.headerIsServed = False
        o = urlparse(self.path)
        qs = parse_qs(o.query)
        global lastUploadCode
        global lastUploadConfig
        global lastUploadFile

        # TODO:
        # if "Android" in self.headers["user-agent"]:
        #     self.htmlDirectory = self.htmlDirectory + "_mobile"

        if o.path == "/" or o.path == "/default":
            self.serveDefault(qs)
        elif o.path == "/motes":
            self.serveMoteSelect(qs)
        elif o.path == "/config":
            self.serveConfig(qs)
        elif o.path == "/graph":
            self.serveGraphs(qs)
        elif o.path == "/graph-data":
            self.serveGraphData(qs)
        elif o.path == "/graph-form":
            self.serveGraphForm(qs)
        elif o.path == "/upload":
            self.serveUploadGet(qs, lastUploadCode, lastUploadConfig, lastUploadFile)
        elif o.path == "/login":
            self.serveLogin(qs)
        elif o.path == "/server":
            self.serveServer(qs)
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
            self.serveFile(os.path.join(self.sealBlocklyDirectory, o.path[14:]), qs)
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
            self.serveFile(os.path.join(self.htmlDirectory, "css", o.path[1:]), qs)
        elif o.path[-4:] in [".png", ".jpg", ".gif", ".tif"]:
            self.serveFile(os.path.join(self.htmlDirectory, "img", o.path[1:]), qs)
        elif o.path[-3:] in [".js"]:
            self.serveFile(os.path.join(self.htmlDirectory, "js", o.path[1:]), qs)
        else:
            self.serve404Error(o.path, qs)   

    def serveBody(self, name, qs = {'sma': ['0000000'],}, replaceValues = None):
        contents = ""
        disabled = "" if self.getLevel() > 1 else 'disabled="disabled" '
        with open(self.htmlDirectory + "/" + name + ".html", "r") as f:
            contents = f.read()
            if replaceValues:
                for v in replaceValues:
                    contents = contents.replace("%" + v + "%", replaceValues[v])
            contents = contents.replace("%DISABLED%", disabled)
            if "sma" in qs: contents = contents.replace("%SMA%", qs["sma"][0])
        return contents

    # Dummy, have to respond somehow, so javascript knows we are here
    def serveSync(self, qs):
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        if self.getLevel() > 1:
            self.writeChunk("writeAccess=True")
        self.writeFinalChunk()

    def serveListenData(self, qs):
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        text = ""
        for line in sensor_data.moteData.listenTxt:
            text += line + "<br/>"
        if text:
            self.writeChunk(text)
        self.writeFinalChunk()

    def do_POST(self):
        self.headerIsServed = False
        o = urlparse(self.path)
        qs = parse_qs(o.query)

# TODO
#        if "Android" in self.headers["user-agent"]:
#            self.htmlDirectory = self.htmlDirectory + "_mobile"

        global lastUploadCode
        global lastUploadConfig
        global lastUploadFile
        
        if o.path == "/upload":
            self.serveUploadPost(qs, lastUploadCode, lastUploadConfig, lastUploadFile)
        else:
            self.serve404Error(o.path, qs)



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
def makeDefaultUserFile(userDirectory, userFile):
    if not os.path.exists(userDirectory):
        os.makedirs(userDirectory)
    uf = open(userDirectory + "/" + userFile, "w")
    for at in configuration.c.getCfgValue("userAttributes"):
        uf.write(at+" ")
    uf.write("\n")
    for ad in configuration.c.getCfgValue("adminValues"):
        uf.write(ad+" ")
    uf.write("\n")
    uf.close()
    return str(userDirectory + "/" + userFile)
    
def readUsers(userDirectory, userFile):
    global allUsers
    uf = open(userDirectory + "/" + userFile,"r")
    i = False
    for line in uf:
         if not i:
             i = True
             allUsers = user.Users(line.split(), userDirectory, userFile)
         else:
             allUsers.add_user(line.split())
    uf.close()
    return i
    
def initalizeUsers():
    global allSessions
    
    allSessions = session.Sessions()
    
    userDirectory = os.path.abspath(configuration.c.getCfgValue("userDirectory"))
    userFile = configuration.c.getCfgValue("userFile")
    if not os.path.exists(userDirectory + "/" + userFile):
        print("No user file. Add default in " + makeDefaultUserFile(userDirectory, userFile))

    if not readUsers(userDirectory, userFile):
        print("User file is empty!")
        print("New default file made in " + makeDefaultUserFile(userDirectory, userFile))
        readUsers(userDirectory, userFile)

    if not "name" in allUsers._userAttributes:
        print("User attribute \"name\" required! Old user file backuped in " + allUsers.make_copy())
        print("New default file made in " + makeDefaultUserFile(userDirectory, userFile))
        readUsers(userDirectory, userFile)
    elif not allUsers.get_user("name", "admin"):
        print("No admin! Old user file backuped in " + allUsers.make_copy())
        print("New default file made in " + makeDefaultUserFile(userDirectory, userFile))
        readUsers(userDirectory, userFile)
    elif not "password" in allUsers._userAttributes:
        print("User attribute \"password\" required! Old user file backuped in " + allUsers.make_copy())
        print("New default file made in " + makeDefaultUserFile(userDirectory, userFile))
        readUsers(userDirectory, userFile)
    elif not allUsers.check_psw():
        print("Passwords do not match the MD5 standard! Old user file backuped in " + allUsers.make_copy())
        print("New default file made in " + makeDefaultUserFile(userDirectory, userFile))
        readUsers(userDirectory, userFile)


    if not allUsers.get_user("name", "admin") or not "name" in allUsers._userAttributes or not "password" in allUsers._userAttributes or not allUsers.check_psw():
        print("There is something wrong with user.cfg")


    ua = configuration.c.getCfgValue("userAttributes")
    na = set(ua) - set(allUsers._userAttributes) #new atributes
    if len(na) > 0:
        dv = configuration.c.getCfgValue("defaultValues")
        av = configuration.c.getCfgValue("adminValues")
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
        print("Save old user file in " + allUsers.make_copy())
        allUsers.write_in_file()


# ---------------------------------------------
def main():
    try:
        if configuration.c.getCfgValueAsBool("createDaemon"):
            # detach from controlling tty and go to background
            daemon.createDaemon()
        # load users
        initalizeUsers()
        # start the server
        port = configuration.c.getCfgValueAsInt("port")
        server = ThreadingHTTPServer(('', port), HttpServerHandler)
        # load motes
        motes.addAll()
        # report ok and enter the main loop
        print("<http-server>: started, listening to TCP port {}, serial baudrate {}".format(port,
              configuration.c.getCfgValueAsInt("baudrate")))
        server.serve_forever()
    except SystemExit:
        raise # XXX
    except Exception as e:
        print("<http-server>: exception occurred:")
        print(e)
        print(traceback.format_exc())
        sys.exit(1)

if __name__ == '__main__':
    main()
