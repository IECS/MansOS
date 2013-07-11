#!/usr/bin/env python

#
# MansOS web server - the main file
#

import os, sys, platform
import threading, time, serial, select, socket, cgi, subprocess, struct, signal
import json
from page_login import *
from page_server import *
from page_account import *
from page_user import *
from page_graph import *
from settings import *
from user import *
from session import *
from mote import *
from sensor_data import *
from config import *
from daemon import *
import re as re
from uuid import getnode as get_mac
import urllib2

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
lastData = ""

isInSubprocess = False
uploadResult = ""

motes = MoteCollection()

sealBlocklyPath = "seal-blockly"

# Packet
usePacketSeparator = False
packetSeparator = "==="
packet = None
sampleRe = re.compile('^\w+=\d+(\.\d+)?(,[\da-fA-F]{2})?$')

connection = None
try:
    from sqlalchemy import create_engine
    from sqlalchemy import Table, Column, MetaData
    from sqlalchemy.types import DateTime, Numeric, Integer, String

    # Database
    url = settingsInstance.getCfgValue("dbServer")
    username = settingsInstance.getCfgValue("dbUsername")
    password = settingsInstance.getCfgValue("dbPassword")
    host = settingsInstance.getCfgValue("dbHost")
    engine = create_engine(url % (username, password, host))
    metadata = MetaData()
    observations = Table('observations', metadata,
        Column('id', Integer, primary_key = True),
        Column('obs_time', DateTime),
        Column('unit_id', String),
        Column('port', String),
        Column('type', String),
        Column('value', Numeric(20,6))
    )
except ImportError:
    print "Warning: database storage dependencies are missing. "
    print "The following packages should be installed to use the database - python-sqlalchemy, python-mysqldb."

saveToDB = False
sendToOpensense = False

def saveDataToDB(packet):
    global connection
    
    mac = str(get_mac())
    for key in packet.keys():
        arr = key.split(":")
        val = packet[key]
        ins = observations.insert().values(obs_time = datetime.datetime.now(), unit_id = mac, port = arr[0], type = arr[1], value = val)
        connection.execute(ins)

def sendDataToSense(packet):
    url = 'http://api.sen.se/events/'
    header = {
      'sense_key' : settingsInstance.getCfgValue("senseApiKey"),
    }
    feeds = settingsInstance.getCfgValue("senseApiFeeds")
    type2feedMap = {}
    for feed in feeds:
        arr = feed.split(":")
        sensorType = arr[0]
        feedId = arr[1]
        type2feedMap[sensorType] = feedId   
    data = []
    #mac = str(get_mac())
    for key in packet.keys():
        arr = key.split(":")
        #port = arr[0]
        sensorType = arr[1]
        value = packet[key]
        measurement = {
            "feed_id": type2feedMap[sensorType],
            "value":   value 
        }
        data.append(measurement)
    data_json = json.dumps(data)
    host = "http://api.sen.se/events/"
    req = urllib2.Request(host, data_json, header)
    response_stream = urllib2.urlopen(req)
    json_response = response_stream.read()
    #print json_response

def processData(port, newString):
    global usePacketSeparator, packet
    global saveToDB, sendToOpensense
    
    if newString == packetSeparator:
        usePacketSeparator = True
        if packet != None:
            if saveToDB == True:
                # Save data to DB
                saveDataToDB(packet)
            if sendToOpensense == True:
                #Send data to sen.se
                sendDataToSense(packet)
            packet.clear()
        else:
            packet = {}
    else:
        s = newString.replace (" ", "")
        if sampleRe.match(s) != None:
            arr = s.split("=")
            if not usePacketSeparator:
                if saveToDB == True:
                    # Save data to DB
                    saveDataToDB({port+":"+arr[0]: arr[1]})
                if sendToOpensense == True:
                    #Send data to sen.se
                    sendDataToSense({port+":"+arr[0]: arr[1]})                               
            else:
                packet[port+":"+arr[0]] = arr[1]
        else:
            print "ERROR: Wrong data format!\n"

def listenSerial():
    global saveToDB, sendToOpensense
    
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
                    if saveToDB == True or sendToOpensense == True:
                        processData(m.port.port, newString)
                    # print "got", newString
                    moteData.addNewData(newString, m.port.portstr)
                m.buffer = m.buffer[pos + 1:]

        moteData.fixSizes()
        # pause for a bit
        time.sleep(0.01)

def closeAllSerial():
    global listenThread
    global isListening
    global connection
    isListening = False
    if listenThread:
        listenThread.join()
        listenThread = None
    for m in motes.getMotes():
        m.closeSerial()
    if connection != None:
        # Close DB connection
        connection.close()

def openAllSerial():
    global listenThread
    global isListening
    global connection
    
    moteData.reset()
    if isListening: return
    isListening = True
    listenThread = threading.Thread(target = listenSerial)
    listenThread.start()
    for m in motes.getMotes():
        m.tryToOpenSerial(False)
    # Open DB connection
    connection = engine.connect()

def getMansosVersion():
    path = settingsInstance.getCfgValue("mansosDirectory")
    result = ""
    try:
        with open(os.path.join(path, "doc/VERSION")) as versionFile:
            result = versionFile.readline().strip()
    except Exception as e:
        print(e)
    return result

# --------------------------------------------
class HttpServerHandler(BaseHTTPRequestHandler, PageUser, PageAccount, PageLogin, PageServer, PageGraph, setAndServeSessionAndHeader):
    server_version = 'MansOS/' + getMansosVersion() + ' Web Server'
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

    def serveHeader(self, name, qs = {"no": "no"}, isGeneric = True, includeBodyStart = True, replaceValues = None, urlTo = ""):
        self.headerIsServed = True
        if name == "default":
            pagetitle = ""
        else:
            pagetitle = " &#8211; " + toTitleCase(name)

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

        if includeBodyStart:
            try:
                if not "no" in qs:
                    self.serveSession(qs, urlTo)
            except Exception as e:
                print("Error Session not served!:")
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

    def serveBody(self, name, qs = {'sma': ['0000000'],}, replaceValues = None):
        disabled = "" if self.getLevel() > 1 else 'disabled="disabled" '
        with open(self.htmlDirectory + "/" + name + ".html", "r") as f:
            contents = f.read()
            if replaceValues:
                for v in replaceValues:
                    contents = contents.replace("%" + v + "%", replaceValues[v])
            contents = contents.replace("%DISABLED%", disabled)
            if "sma" in qs: contents = contents.replace("%SMA%", qs["sma"][0])
            self.writeChunk(contents)


    def serveMotesNew(self, action, namedAction, qs, isPost):
        text = ''
        disabled = "" if self.getLevel() > 1 else 'disabled="disabled" '
        i = 0
        for m in motes.getMotes():
            name = "mote" + str(i)

            if name in qs:
                m.isSelected = qs[name][0] == 'on'
            elif "action" in qs:
                m.isSelected = False

            checked = ' checked="checked"' if m.isSelected else ""

            text += '<div class="mote"><strong>Mote: </strong>' + m.portName
            text += ' (<strong>Platform: </strong>' + m.platform + ') '
            text += ' <input type="checkbox" title="Select the mote" name="' + name + '"'
            text += checked + ' ' + disabled + '/>' + namedAction + '</div>\n'
            i += 1

        # remember which motes were selected and which were not
        motes.storeSelected()

        if text:
            text = '<div class="motes1">\nDirectly attached motes:\n<br/>\n' + text + '</div>\n'
        return text

    def serveMotes(self, action, namedAction, qs, isPost, writeChunk=True):
        text = ''
        if isPost:
            text += '<form method="post" enctype="multipart/form-data" action="' + action + '">'
        else:
            text += '<form action="' + action + '">'
        self.writeChunk(text)
        disabled = "" if self.getLevel() > 1 else 'disabled="disabled" '
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
            if writeChunk == True:
                self.writeChunk(c)
        self.writeChunk('<div class="form">\n')


    def serveMoteMotes(self, qs):
        if motes.isEmpty():
            self.serveError("No motes connected!")
            return

        text = '<form action="config"><div class="motes2">\n'
        text += 'Directly attached motes:<br/>\n'
        
        disabled = "" if self.getLevel() > 1 else 'disabled="disabled" '

        i = 0
        for m in motes.getMotes():
            name = "mote" + str(i)
            text += '<div class="mote"><strong>Mote: </strong>' + m.portName
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
        with open(self.htmlDirectory + "/footer.html", "r") as f:
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
        if filename[-4:] == '.css':
            mimetype = 'text/css'
            if filename[-9:] == 'theme.css':
                tpath = filename[:-4]
                filename = filename[:-4] + settingsInstance.getCfgValue("serverTheme") + '.css'
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

    def serveListen(self, qs):
        global saveToDB
        global sendToOpensense
        
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("listen", qs)
        #self.serveMotes("listen", "Listen", qs, False, False)

        motesText = self.serveMotesNew("listen", "Listen", qs, False)
        if "action" in qs and self.getLevel() > 1:
            if qs["action"][0] == "Start":
                if not motes.anySelected():
                    self.serveError("No motes selected!", False)
                if isListening:
                    self.serveError("Already listening!", False)
                saveToDB = False
                sendToOpensense = False
                if ("saveToDB" in qs and qs["saveToDB"][0] == 'on'):
                    saveToDB = True
                if ("sendToOpensense" in qs and qs["sendToOpensense"][0] == 'on'):
                    sendToOpensense = True                    
                openAllSerial()
            else:
                closeAllSerial()

        txt = ""
        for line in moteData.listenTxt:
            txt += line + "<br/>"

        action = "Stop" if isListening else "Start"
        
        dataFilename = settingsInstance.getCfgValue("saveToFilename")
        saveProcessedData = settingsInstance.getCfgValueAsBool("saveProcessedData")
        
        if self.getLevel() > 1:
            if "dataFile" in qs:
                dataFilename = qs["dataFile"][0]
                if len(dataFilename) and dataFilename.find(".") == -1:
                    dataFilename += ".csv"
            if "dataType" in qs:
                saveProcessedData = not qs["dataType"][0] == "raw"
                saveMultipleFiles = qs["dataType"][0] == "mprocessed"

            settingsInstance.setCfgValue("saveToFilename", dataFilename)
            settingsInstance.setCfgValue("saveProcessedData", bool(saveProcessedData))
            settingsInstance.save()
        
        rawdataChecked = not saveProcessedData
        mprocessedChecked = saveProcessedData

        div_height = "0"
        if "div_height" in qs:
            div_height = qs["div_height"][0]
        self.serveBody("listen", qs,
                       {"MOTES_TXT" : motesText,
                        "LISTEN_TXT" : txt,
                        "MOTE_ACTION": action,
                        "DATA_FILENAME" : dataFilename,
                        "RAWDATA_CHECKED" : 'checked="checked"' if rawdataChecked else "",
                        "MPROCDATA_CHECKED" : 'checked="checked"' if mprocessedChecked else "",
                        "DIV_HEIGHT" : div_height})
        self.serveFooter()

    def serveUpload(self, qs):
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        self.serveHeader("upload", qs)
        #self.serveMotes("upload", "Upload", qs, True)
        motesText = self.serveMotesNew("listen", "Listen", qs, True)
        isSealCode = settingsInstance.getCfgValueAsInt("isSealCode")
        isSlow = settingsInstance.getCfgValueAsInt("slowUpload")
        self.serveBody("upload", qs,
                       {"MOTES_TXT" : motesText,
                        "CCODE_CHECKED": 'checked="checked"' if not isSealCode else "",
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
        #if self.getLevel() < 2:
        #    self.serveDefault(qs)
        isInSubprocess = True
        try:
            self.setSession(qs)
            self.send_response(200)
            self.sendDefaultHeaders()
            self.end_headers()
            self.serveHeader("upload", qs)
            self.writeChunk('<button type="button" onclick="window.open(\'\', \'_self\', \'\'); window.close();">OK</button><br/>')
            self.writeChunk("Upload result:<br/><pre>\n")
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
            disabled = 'disabled="disabled"' if not self.getLevel() > 1 else ""
            contents = contents.replace("%DISABLED%", disabled)
            self.writeChunk(contents)
        self.writeFinalChunk()

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
        for line in moteData.listenTxt:
            text += line + "<br/>"
        if text:
            self.writeChunk(text)
        self.writeFinalChunk()

    def do_GET(self):
        #global
        self.sessions = allSessions
        self.users = allUsers
        self.settings = settingsInstance
        self.tabuList = tabuList
        self.moteData = moteData
        #global end
        
        self.headerIsServed = False

        o = urlparse(self.path)
        qs = parse_qs(o.query)

        self.htmlDirectory = htmlDirectory
        if "Android" in self.headers["user-agent"]:
            self.htmlDirectory = self.htmlDirectory+"_mobile"
            
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
        elif o.path == "/graph-form":
            self.serveGraphsForm(qs)
        elif o.path == "/upload":
            self.serveUpload(qs)
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
            self.serveFile(self.htmlDirectory + "/css/" + o.path)
        elif o.path[-4:] in [".png", ".jpg", ".gif", ".tif"]:
            self.serveFile(self.htmlDirectory + "/img/" + o.path)
        elif o.path[-3:] in [".js"]:
            self.serveFile(self.htmlDirectory + "/js/" + o.path)
        else:
            self.serve404Error(o.path, qs)

    def compileAndUpload(self, code, config, fileContents, isSEAL):
        global lastUploadCode
        global lastUploadConfig
        global lastUploadFile
        #if self.getLevel < 2:
        #    return 1
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
        self.serveMotes("upload", "Upload", {}, True)
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
def makeDefaultUserFile(userDirectory, userFile):
    if not os.path.exists(userDirectory):
        os.makedirs(userDirectory)
    uf = open(userDirectory + "/" + userFile, "w")
    for at in settingsInstance.getCfgValue("userAttributes"):
        uf.write(at+" ")
    uf.write("\n")
    for ad in settingsInstance.getCfgValue("adminValues"):
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
             allUsers = Users(line.split(), userDirectory, userFile)
         else:
             allUsers.add_user(line.split())
    uf.close()
    return i
    
def initalizeUsers():
    global allSessions
    
    allSessions = Sessions()
    
    userDirectory = os.path.abspath(settingsInstance.getCfgValue("userDirectory"))
    userFile = settingsInstance.getCfgValue("userFile")
    if not os.path.exists(userDirectory + "/" + userFile):
        print("No user file. Python add default in " + makeDefaultUserFile(userDirectory, userFile))
        
    
    if not readUsers(userDirectory, userFile):
        print("User file is empty!")
        print("New default file made in " + makeDefaultUserFile(userDirectory, userFile))
        readUsers(userDirectory, userFile)
    
    if not "name" in allUsers._userAttributes:
        print("User attribute \"name\" required! Python save old user file in " + allUsers.make_copy())
        print("New default file made in " + makeDefaultUserFile(userDirectory, userFile))
        readUsers(userDirectory, userFile)
    elif not allUsers.get_user("name", "admin"):
        print("No admin! Python save old user file in " + allUsers.make_copy())
        print("New default file made in " + makeDefaultUserFile(userDirectory, userFile))
        readUsers(userDirectory, userFile)
    elif not "password" in allUsers._userAttributes:
        print("User attribute \"password\" required! Python save old user file in " + allUsers.make_copy())
        print("New default file made in " + makeDefaultUserFile(userDirectory, userFile))
        readUsers(userDirectory, userFile)
    elif not allUsers.check_psw():
        print("Passwords do not match the md5 standard! Python save old user file in " + allUsers.make_copy())
        print("New default file made in " + makeDefaultUserFile(userDirectory, userFile))
        readUsers(userDirectory, userFile)


    if not allUsers.get_user("name", "admin") or not "name" in allUsers._userAttributes or not "password" in allUsers._userAttributes or not allUsers.check_psw():
        print("There is something wrong with user.cfg")

    
    ua = settingsInstance.getCfgValue("userAttributes")
    na = set(ua) - set(allUsers._userAttributes) #new atributes
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
        print("Python save old user file in " + allUsers.make_copy())
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
