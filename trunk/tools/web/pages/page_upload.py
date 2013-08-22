import threading, time, cgi, os
import configuration
import helper_tools as ht
from motes import motes
import utils
import sensor_data

maybeIsInSubprocess = False
isPostEntered = False
subprocessLock = threading.Lock()

class PageUpload():
    
    def serveUploadGet(self, qs, lastUploadCode, lastUploadConfig, lastUploadFile):
        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()

        motesText = self.serveMotes("upload", "Upload", qs, None)
        isSealCode = configuration.c.getCfgValueAsBool("isSealCode")
        isSlow = configuration.c.getCfgValueAsBool("slowUpload")
        self.serveAnyPage("upload", qs, True, {"MOTES_TXT" : motesText,
                        "CCODE_CHECKED": 'checked="checked"' if not isSealCode else "",
                        "SEALCODE_CHECKED" : 'checked="checked"' if isSealCode else "",
                        "UPLOAD_CODE" : lastUploadCode,
                        "UPLOAD_CONFIG" : lastUploadConfig,
                        "UPLOAD_FILENAME": lastUploadFile,
                        "SLOW_CHECKED" : 'checked="checked"' if isSlow else ""})
        
    def serveUploadPost(self, qs, lastUploadCode, lastUploadConfig, lastUploadFile):
        global maybeIsInSubprocess
        global isPostEntered

        self.setSession(qs)

        # Parse the form data posted
        form = cgi.FieldStorage(
            fp = self.rfile,
            headers = self.headers,
            environ = {'REQUEST_METHOD':'POST',
                     'CONTENT_TYPE':self.headers['Content-Type'],
                     })

        # signal the upload result thread that we are are ready to start serving
        isPostEntered = True

        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.send_header('Transfer-Encoding', 'chunked')
        self.end_headers()

        file_data = None

        isSEAL = False
        if "compile" in form:
            if "language" in form:
                isSEAL = form["language"].value.strip() == "SEAL"
            configuration.c.setCfgValue("isSealCode", isSEAL)

        if "slow" in form:
            slow = form["slow"].value == "on"
        else:
            slow = False
        configuration.c.setCfgValue("slowUpload", slow)
        configuration.c.save()

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
            self.serveHeader("upload", qs)
            text = "Neither filename nor code specified!"
            maybeIsInSubprocess = False
            self.serveAnyPage("error:critical", qs, errorMsg = text)
            return

        for m in motes.getMotes():
            name = "mote" + m.getPortBasename()
            if name in form:
                isChecked = form[name].value == "on"
            else:
                isChecked = False

            if isChecked:
                m.isSelected = True
            else:
                m.isSelected = False

        # remember which motes were selected and which not
        motes.storeSelected()
        # check if any motes are selected
        if not motes.anySelected():
            self.serveHeader("upload", qs)
            text = "No motes selected!"
            maybeIsInSubprocess = False
            self.serveAnyPage("error:critical", qs, errorMsg = text)         
            return

        config = ""
        if "config" in form.keys():
            lastUploadConfig = form["config"].value
            config = lastUploadConfig
        if slow:
            config += "\nSLOW_UPLOAD=y\n"

        retcode = self.compileAndUpload(code, config, fileContents, isSEAL)
        motesText = self.serveMotes("upload", "Upload", None, form)
        isSealCode = configuration.c.getCfgValueAsBool("isSealCode")
        isSlow = configuration.c.getCfgValueAsBool("slowUpload")
        if retcode == 0:
            infoMsg = "<div><strong>Upload done!</strong></div><br/>"
        else:
            infoMsg = "<div><strong>Upload failed!</strong></div><br/>"
            
        self.serveAnyPage("upload", qs, True, {"MOTES_TXT" : motesText,
                        "CCODE_CHECKED": 'checked="checked"' if not isSealCode else "",
                        "SEALCODE_CHECKED" : 'checked="checked"' if isSealCode else "",
                        "UPLOAD_CODE" : lastUploadCode,
                        "UPLOAD_CONFIG" : lastUploadConfig,
                        "UPLOAD_FILENAME": lastUploadFile,
                        "SLOW_CHECKED" : 'checked="checked"' if isSlow else ""}, infoMsg = infoMsg)

    def serveUploadResult(self, qs):
        global maybeIsInSubprocess
        global isPostEntered

        #if self.getLevel() < 2:
        #    self.serveDefault(qs)
        inFileName = os.path.join("build", "child_output.txt")
        inFile = None

        maybeIsInSubprocess = True
        startWait = time.time()
        while not isPostEntered:
            now = time.time()
            # wait a maximum of 10 seconds
            if now - startWait > 10.0:
                break
        isPostEntered = False

        try:
            #self.setSession(qs)
            self.send_response(200)
            self.sendDefaultHeaders()
            self.end_headers()
            qs["no"] = "no"
            self.serveHeaderOld("upload", qs)
            self.writeChunk('<button type="button" onclick="window.open(\'\', \'_self\', \'\'); window.close();">OK</button><br/>')
            self.writeChunk("Upload result:<br/><pre>\n")

            # wait until subprocess output file appears
            while maybeIsInSubprocess or inFile == None:
                try:
                    inFile = open(inFileName, "rb")
                except:
                    inFile = None
                    time.sleep(0.001)

            if inFile:
#                print("in file opened ok")
                uploadLine = ""
                while True:
                    if utils.fileIsOver(inFile):
                        if maybeIsInSubprocess:
                            time.sleep(0.001)
                            continue
                        else:
                            break
                    # read one symbol
                    c = inFile.read(1)
                    uploadLine += c
                    if c == '\n':
                        # if newline reached, print out the current line
                        self.writeChunk(uploadLine)
                        uploadLine = ""
                # write final chunk
                if uploadLine:
                    self.writeChunk(uploadLine)
            self.writeChunk("</pre>\n")
        except:
            raise
        finally:
            # clean up
            try:
                if inFile: inFile.close()
                os.remove(inFileName)
            except:
                pass
#            print("upload result served!")
            uploadResult = ""
            
    def compileAndUpload(self, code, config, fileContents, isSEAL):
        global lastUploadCode
        global lastUploadConfig
        global lastUploadFile
        global maybeIsInSubprocess

        if not os.path.exists("build"):
            os.mkdir("build")

        # do this synchronously
        subprocessLock.acquire()
        global isListening
        ht.closeAllSerial()
        isListening = False
        maybeIsInSubprocess = True
        try:
           if fileContents:
               lastUploadFile = form["file"].filename

               filename = os.path.join("build", "tmp-file.ihex")
               with open(filename, "w") as outFile:
                   outFile.write(fileContents)
                   outFile.close()

               retcode = 0
               for m in motes.getMotes():
                   if not m.tryToOpenSerial(False): continue
                   r = m.tryToUpload(self, filename)
                   if r != 0: retcode = r

           elif code:
               lastUploadCode = code

               filename = "main." + ("sl" if isSEAL else "c")
               with open(os.path.join("build", filename), "w") as outFile:
                   outFile.write(code)
                   outFile.close()

               with open(os.path.join("build", "config"), "w") as outFile:
                   if config is None:
                       config = ""
                   outFile.write(config)
                   outFile.close()

               with open(os.path.join("build", "Makefile"), "w") as outFile:
                   if isSEAL:
                       outFile.write("SEAL_SOURCES = main.sl\n")
                   else:
                       outFile.write("SOURCES = main.c\n")
                   outFile.write("APPMOD = App\n")
                   outFile.write("PROJDIR = $(CURDIR)\n")
                   outFile.write("ifndef MOSROOT\n")
                   mansosPath = configuration.c.getCfgValue("mansosDirectory")
                   if not os.path.isabs(mansosPath):
                       # one level up - because we are in build directory
                       mansosPath = os.path.join(mansosPath, "..")
                   outFile.write("  MOSROOT = " + mansosPath + "\n")
                   outFile.write("endif\n")
                   outFile.write("include ${MOSROOT}/mos/make/Makefile\n")
                   outFile.close()

               retcode = 0
               for m in motes.getMotes():
                   if not m.tryToOpenSerial(False): continue
                   r = m.tryToCompileAndUpload(self, filename)
                   if r != 0: retcode = r

        finally:
            maybeIsInSubprocess = False
            global isListening
            sensor_data.moteData.reset()
            ht.openAllSerial()
            isListening = True
            subprocessLock.release()
            return retcode

        