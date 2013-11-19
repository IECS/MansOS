import threading, time, cgi, os
import configuration
import helper_tools as ht
from motes import motes
import utils
import sensor_data

maybeIsInSubprocess = False
forceInterrupt = False
subprocessLock = threading.Lock()

def emitCodeMansOS(code, config):
    with open(os.path.join("build", "main.c"), "w") as outFile:
        outFile.write(code)
        outFile.close()

    with open(os.path.join("build", "config"), "w") as outFile:
        outFile.write(config)
        outFile.close()

    with open(os.path.join("build", "Makefile"), "w") as outFile:
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


def emitCodePlainC(code, config):
    with open(os.path.join("build", "main.c"), "w") as outFile:
        outFile.write(code)
        outFile.close()

    with open(os.path.join("build", "config"), "w") as outFile:
        # do not use mansos main in this case
        config += "\n"
        config += "USE_KERNEL_MAIN=n\n"
        config += "USE_HARDWARE_TIMERS=n\n"
        outFile.write(config)
        outFile.close()

    with open(os.path.join("build", "Makefile"), "w") as outFile:
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


def emitCodeTinyOS(code, config):
    with open(os.path.join("build", "MyC.nc"), "w") as outFile:
        outFile.write(code)
        outFile.close()

    with open(os.path.join("build", "MyAppC.nc"), "w") as outFile:
        outFile.write(config)
        outFile.close()

    with open(os.path.join("build", "Makefile"), "w") as outFile:
        outFile.write("COMPONENT = MyAppC\n")
        outFile.write("include $(MAKERULES)\n")
        outFile.close()


def emitCodeContiki(code):
    with open(os.path.join("build", "app.c"), "w") as outFile:
        outFile.write(code)
        outFile.close()

    with open(os.path.join("build", "Makefile"), "w") as outFile:
        outFile.write("CONTIKI_PROJECT = app\n")
        outFile.write("all: $(CONTIKI_PROJECT)\n")
        outFile.write("PROJDIR = $(CURDIR)\n")
        contikiPath = configuration.c.getCfgValue("contikiDirectory")
        if not os.path.isabs(contikiPath):
            # one level up - because we are in build directory
            contikiPath = os.path.join(contikiPath, "..")
        outFile.write("CONTIKI = " + contikiPath + "\n")
        outFile.write("include $(CONTIKI)/Makefile.include\n")
        outFile.close()


def emitCodeSEAL(code, config):
    with open(os.path.join("build", "main.sl"), "w") as outFile:
        outFile.write(code)
        outFile.close()

    with open(os.path.join("build", "config"), "w") as outFile:
        outFile.write(config)
        outFile.close()

    with open(os.path.join("build", "Makefile"), "w") as outFile:
        outFile.write("SEAL_SOURCES = main.sl\n")
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


class PageUpload():
    def serveUploadGet(self, qs): #, lastUploadCode, lastUploadConfig, lastUploadFile):
        global isListening

        self.setSession(qs)
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()

        # TODO
        lastUploadCode = ""
        lastUploadConfig = ""
        lastUploadFile = ""

        motesText = self.serveMotes("upload", "Upload", qs, None)
        codeType = configuration.c.getCfgValue("codeType").lower()
        isSlow = configuration.c.getCfgValueAsBool("slowUpload")
        self.serveAnyPage("upload", qs, True, {"MOTES_TXT" : motesText,
                        "CCODE_SELECTED": 'selected="selected"' if codeType == "c" else "",
                        "PLAINCCODE_SELECTED": 'selected="selected"' if codeType == "plain_c" else "",
                        "NESCCODE_SELECTED": 'selected="selected"' if codeType == "nesc" else "",
                        "CONTIKICODE_SELECTED": 'selected="selected"' if codeType == "contiki_c" else "",
                        "SEALCODE_SELECTED": 'selected="selected"' if codeType == "seal" else "",
                        "UPLOAD_CODE" : lastUploadCode,
                        "UPLOAD_CONFIG" : lastUploadConfig,
                        "UPLOAD_FILENAME": lastUploadFile,
                        "SLOW_CHECKED" : 'checked="checked"' if isSlow else ""})

    def serveUploadPost(self, qs): #, lastUploadCode, lastUploadConfig, lastUploadFile):
        global maybeIsInSubprocess
        global forceInterrupt

        # TODO
        lastUploadCode = ""
        lastUploadConfig = ""
        lastUploadFile = ""

        # Parse the form data posted
        form = cgi.FieldStorage(
            fp = self.rfile,
            headers = self.headers,
            environ = {
                'REQUEST_METHOD':'POST',
                'CONTENT_TYPE':self.headers['Content-Type']
            })

        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        
        file_data = None

        codeType = "C"
        if "compile" in form:
            if "language" in form:
                codeType = form["language"].value.strip().lower()
            configuration.c.setCfgValue("codeType", codeType)

        if "slow" in form:
            slow = form["slow"].value == "on"
        else:
            slow = False
        configuration.c.setCfgValue("slowUpload", slow)
        configuration.c.save()

        code = None
        fileContents = None
        fileName = None

        if "compile" in form and "code" in form:
            code = form["code"].value
        if "upload" in form and "file" in form:
            fileContents = form["file"].file.read()
            fileName = form["file"].filename

        # check if what to upload is provided
        if not fileContents and not code:
            infoMsg = "Neither filename nor code specified!"
            maybeIsInSubprocess = False
            forceInterrupt = True
            self.writeChunk(infoMsg);
            return

        for m in motes.getMotes():
            name = "mote" + m.getFullBasename()
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
            infoMsg = "No motes selected!"
            maybeIsInSubprocess = False
            forceInterrupt = True
            self.writeChunk(infoMsg);
            return

        config = ""
        if "config" in form.keys():
            lastUploadConfig = form["config"].value
            config = lastUploadConfig
        if slow:
            config += "\nSLOW_UPLOAD=y\n"

        retcode = self.compileAndUpload(code, config, fileName, fileContents, codeType)
        motesText = self.serveMotes("upload", "Upload", None, form)
        codeType = configuration.c.getCfgValue("codeType")
        isSlow = configuration.c.getCfgValueAsBool("slowUpload")
        if retcode == 0:
            infoMsg = "Upload done!"
        else:
            infoMsg = "Upload failed!"
        self.writeChunk(infoMsg);

    def serveUploadResult(self, qs):
        global maybeIsInSubprocess
        global forceInterrupt

        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()

        inFileName = os.path.join("build", "child_output.txt")
        inFile = None

        try:
            limit = int(qs['line'][0])
        except:
            limit = 100000

        if limit == 1:
            # first time in this function
            maybeIsInSubprocess = True

        uploadLine = ""
        try:
            # wait until subprocess output file appears
            while inFile == None:
                try:
                    if forceInterrupt:
                        self.writeChunk("Finished!")
                        forceInterrupt = False
                        return
                    inFile = open(inFileName, "rb")
                except:
                    inFile = None
                    time.sleep(0.001)

            if inFile:
                i = 0;
                while True:
                    if utils.fileIsOver(inFile):
                        if maybeIsInSubprocess:
                            time.sleep(0.001)
                            continue
                        else:
                            self.writeChunk("Finished!")
                            # clean up
                            try:
                                if inFile: inFile.close()
                                os.remove(inFileName)
                            except:
                                pass
                            break
                    # read one symbol
                    c = inFile.read(1)
                    uploadLine += c
                    if c == '\n':
                        # if newline reached, print out the current line
                        self.writeChunk(uploadLine)
                        uploadLine = ""
                        i = i + 1
                        if i > limit:
                            break;
        except:
            raise

    def compileAndUpload(self, code, config, fileName, fileContents, codeType):
#        global lastUploadCode
#        global lastUploadConfig
#        global lastUploadFile
        global maybeIsInSubprocess
        global isListening

        if not os.path.exists("build"):
            os.mkdir("build")

        # do this synchronously
        subprocessLock.acquire()
        ht.closeAllSerial()
        isListening = False
        maybeIsInSubprocess = True
        try:
           if fileContents:
               lastUploadFile = fileName

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

               if config == None:
                   config = ""

               if codeType == "c":
                   emitCodeMansOS(code, config)
               elif codeType == "plain_c":
                   emitCodePlainC(code, config)
               elif codeType == "nesc":
                   emitCodeTinyOS(code, config)
               elif codeType == "contiki_c":
                   emitCodeContiki(code)
               elif codeType == "seal":
                   emitCodeSEAL(code, config)
               else:
                   print("compileAndUpload: unknow code type: " + codeType)
                   return 1

               retcode = 0
               for m in motes.getMotes():
                   if m.isLocal():
                       if not m.tryToOpenSerial(False): continue
                   r = m.tryToCompileAndUpload(self, codeType)
                   if r != 0: retcode = r

        finally:
            maybeIsInSubprocess = False
            sensor_data.moteData.reset()
            ht.openAllSerial()
            isListening = True
            subprocessLock.release()
            return retcode
