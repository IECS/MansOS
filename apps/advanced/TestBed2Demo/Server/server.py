#!/usr/bin/env python

import os, platform, urlparse, BaseHTTPServer, threading, time, serial, select, socket, cgi, subprocess, struct

#REFERENCE_VOLTAGE = 2.5
REFERENCE_VOLTAGE = 3

HTTP_SERVER_PORT = 8080

TOTAL_MOTES = 1
BAUDRATE = 38400

# params: on/off, led index
CMD_LED_CONTROL  = 1
# params: channel, sampling period in ms
CMD_ADC_CONTROL  = 2
# params: channel, constant value
CMD_DAC_CONTROL  = 3
# params: enable/disable
CMD_SERIAL_CONTROL  = 4
# params: enable/disable
CMD_SD_CARD_CONTROL  = 5

START_CHARACTER = '$'

activeLeds = [False, False, False, False]


NONE_MODULE        = 0
SIGNAL_MODULE      = 1
POWER_MODULE       = 2
MOTE_MODULE        = 3 # not a module, but the attached mote

activeModule = SIGNAL_MODULE


listenThread = None

adcChannel = 0
adcPeriod = 1000
writeSerial = True
writeSD = False

writebuffer = ""

class SerialPort:
    number = 0
    handle = None
    buffer = []
    writebuffer = ""

ports = []
doQuit = False
#inCmdSend = False

#MAGIC_KEY          = 0xD19A
MAGIC_KEY           = 0x9A
KEY_LEN             = 8

defaultComPort = "COM13"

def convertToVolts(value):
    return value / float(4095) * REFERENCE_VOLTAGE

def getComPortString(motenr):
    global defaultComPort
    if platform.system()[:6] == 'CYGWIN':
        return defaultComPort
    if platform.system()[:7] == 'Windows':
        return defaultComPort
    return "/dev/ttyUSB%d" % motenr

def openSerial(motenr, port):
    port.number = motenr
    try:
        port.handle = serial.Serial(getComPortString(motenr), BAUDRATE, timeout=1,
                                    parity=serial.PARITY_NONE, rtscts=1)
        port.handle.setDTR(0)
        port.handle.setRTS(0)
    except Exception, e:
        print "\nSerial exception:\n\t", e
        return
    
    print "Listening to serial port: ", port.handle.portstr, ", rate: ", BAUDRATE


def sendCommand(portNr, command, arg1 = None, arg2 = None):
#    global inCmdSend

#    inCmdSend = True
#    print "before cmd: ", ser.read()

#    global writebuffer
    
    port = ports[portNr]

    port.writebuffer = ""
    port.writebuffer += START_CHARACTER
    port.writebuffer += struct.pack('B', command)
    if command == CMD_ADC_CONTROL:
        args = struct.pack('<BBI', 5, arg1, arg2)
    elif command == CMD_SERIAL_CONTROL \
            or command == CMD_SD_CARD_CONTROL:
        args = struct.pack('BB', 1, arg1)
    elif command == CMD_LED_CONTROL:
        args = struct.pack('BBB', 2, arg1, arg2)
    else:
        args = None

    crc = ord(START_CHARACTER) ^ command
    if args:
        for a in args:
            crc ^= ord(a)
        port.writebuffer += args
    port.writebuffer += struct.pack('B', crc)
#    print "after cmd: ", ser.read()
#    inCmdSend = False
#    writebuffer = port.writebuffer
#    print "writebuffer set to", writebuffer

    port.handle.write(port.writebuffer)
    port.handle.flush()
    port.writebuffer = ""
    print "cmd sent!"


def updateConfig(adcChannel, adcPeriod, writeSerial, writeSD):
    if not ports[0].handle:
        return False

    sendCommand(0, CMD_ADC_CONTROL, adcChannel, adcPeriod)
    sendCommand(0, CMD_SERIAL_CONTROL, writeSerial)
    sendCommand(0, CMD_SD_CARD_CONTROL, writeSD)

    return True

def updateActiveLeds(newActiveLeds):
    global activeLeds

    print "updateActiveLeds!"

    if not ports[0].handle:
        return False

    print "updateActiveLeds: send commands"
    for i in range(4):
#        if newActiveLeds[i] != activeLeds[i]:
            sendCommand(0, CMD_LED_CONTROL, i, int(newActiveLeds[i]))
    activeLeds = newActiveLeds
    print "done"
    return True

def updateActiveModule(newActiveModule):
    global activeModule

    if not ports[0].handle:
        return False

    ser = ports[0].handle
    activeModule = newActiveModule
    print "selecting module", activeModule
    # send magic key
    for i in range(KEY_LEN):
        # clock low
        ser.setRTS(0)
        time.sleep(0.001)
        # data
        ser.setDTR(not bool(MAGIC_KEY & (1 << (KEY_LEN - 1 - i))))
#        print "key bit ", bool(MAGIC_KEY & (1 << (KEY_LEN - 1 - i)))
        # clock high
        ser.setRTS(1)
        time.sleep(0.001)
    # send address
    for i in range(2):
        # clock low
        ser.setRTS(0)
        time.sleep(0.001)
        # data
        ser.setDTR(not bool(activeModule & (1 << (1 - i))))
#        print "data bit ", bool(module & (1 << (1 - i)))
        # clock high
        ser.setRTS(1)
        time.sleep(0.001)
    print "done!"
    ser.setDTR(0)
    ser.setRTS(0)
    return True


def listenSerial():
    global ports
    global doQuit
    global inCmdSend
    global writebuffer
    for i in range(len(ports)):
        openSerial(i, ports[i])

#    udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#    udpsock.bind(('', HTTP_SERVER_NOTIFICATON_PORT))

    while not doQuit:
        p = ports[0]

#        if writebuffer:
#            print "write..."
#            p.handle.write(writebuffer)
#            p.handle.flush()
#            writebuffer = ""

        value = ""
        while True:
            c = p.handle.read(1)
            value += c
            if c == ' ' or c == '\t' or c == '\n' or c == '\r':
                break

        try:
            value = int(value)
            p.buffer.append(value)
            if len(p.buffer) > 100:
                p.buffer = p.buffer[1:]
        except:
            pass

#            p.writebuffer = p.writebuffer[length:]
#            fd.flush()

#        readfd = []
#        errfd = []
#        writefd = []

#        for p in ports:
#            if p.handle != None:
#                readfd.append(p.handle)
#                errfd.append(p.handle)
#                if p.writebuffer:
#                    writefd.append(p.handle)

#        print "select..."
#        (readfd, writefd, errfd) = select.select(readfd, writefd, errfd, 1.0)

#        for fd in readfd:
#            buf = fd.read(1)
#            for p in ports:
#                if p.handle == fd:
#                    p.buffer += buf
#                    break

#        for fd in writefd:
#            length = fd.write(p.writebuffer)
#            print "<http-server>: wrote {} bytes...".format(length)
#            p.writebuffer = p.writebuffer[length:]
#            fd.flush()

#        for fd in readfd:
#            value = ""
#            while True:
#                c = fd.read(1)
#                value += c
#                if c == ' ' or c == '\t' or c == '\n':
#                    break
#            try:
#                value = int(value)
#                p.buffer.append(value)
#                if len(p.buffer) > 100:
#                    p.buffer = p.buffer[1:]
#            except:
#               pass

#        for fd in errfd:
#            for p in ports:
#                if p.handle == fd:
#                    print "<http-server>: closing a serial port, there was an error..."
#                    fd.close()
#                    p.handle = None

# --------------------------------------------

def closeAllSerial():
    global ports
    global doQuit
    global listenThread
    doQuit = True
    listenThread.join()
    for p in ports:
        p.buffer = []
        if p.handle:
            p.handle.close()
            p.handle = None

def openAllSerial():
    global listenThread
    global doQuit
    doQuit = False
    listenThread = threading.Thread(target=listenSerial)
    listenThread.start()


# --------------------------------------------

class HttpServerHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    server_version = 'TestBedAccess Web Server/0.1'
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

    def serveHeader(self, name, replaceValues = None):
        with open("header.html", "r") as f:
            contents = f.read()
            contents = contents.replace("%PAGETITLE%", name)
            self.writeChunk(contents)
        try:
            with open(name + ".header.html", "r") as f:
                contents = f.read()
                if replaceValues:
                    for v in replaceValues:
                        contents = contents.replace(v, replaceValues[v])
                self.writeChunk(contents)
        except:
            pass

        with open("bodystart.html", "r") as f:
            contents = f.read()
            self.writeChunk(contents)

        with open(name + ".html", "r") as f:
            contents = f.read()
            if replaceValues:
                for v in replaceValues:
                    contents = contents.replace(v, replaceValues[v])
            self.writeChunk(contents)


    def serveFooter(self):
        with open("footer.html", "r") as f:
            contents = f.read()
            self.writeChunk(contents)
        self.writeFinalChunk()

    def serveDefault(self):
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.send_header('Transfer-Encoding', 'chunked')
        self.end_headers()
        self.serveHeader("default")
        self.serveFooter()

    def serveGraphs(self, qs):
        global ports
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.send_header('Transfer-Encoding', 'chunked')
        self.send_header('Refresh', '0.5; /graphs')
        self.end_headers()

        doReset = False
        if "reset" in qs:
            doReset = True

        pairs = ""
        # TODO: support multiple ports!
        for p in ports:
            if doReset:
                p.buffer = []
    
            if not p.buffer:
                continue

            i = 0
            for v in p.buffer:
                pairs += "['{}', {}], ".format(i, convertToVolts(v))
                i += 1
            break

        if len(pairs) == 0:
            pairs = "['0', 0]"
        else:
            pairs = pairs[:-2]
        self.serveHeader("graphs", {"%VALUES%" : pairs})

        self.serveFooter()

    def serveConfig(self, qs):
        global adcChannel
        global adcPeriod
        global writeSD
        global writeSerial

        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.send_header('Transfer-Encoding', 'chunked')
        self.end_headers()

        if qs:
            try:
                adcChannel = int(qs["adcChannel"][0])
            except:
                self.writeChunk("<strong>Wrong ADC channel format!</strong><br/><br/>")
                adcChannel = 0
            try:
                adcPeriod = int(qs["adcPeriod"][0])
            except:
                self.writeChunk("<strong>Wrong ADC period format!</strong><br/><br/>")
                adcPeriod = 1000
            writeSerial = qs.get("writeSerial", ["off"])
            writeSerial = True if writeSerial[0] == 'on' else False
            writeSD = qs.get("writeSD", ["off"])
            writeSD = True if writeSD[0] == 'on' else False

            updateConfig(adcChannel, adcPeriod, writeSerial, writeSD)
            self.writeChunk("<p><strong>Values updated!</strong></p>")

        self.serveHeader("config", {
                "%ADC_CHANNEL%" : str(adcChannel),
                "%ADC_PERIOD%" : str(adcPeriod),
                "%WRITE_SERIAL%" : "checked" if writeSerial else "",
                "%WRITE_SD%" : "checked" if writeSD else ""
                })
        self.serveFooter()

    def serveModuleConfig(self, qs):
        global activeModule

        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.send_header('Transfer-Encoding', 'chunked')
        self.end_headers()

        newActiveModule = activeModule

        if qs:
            newActiveModule = qs.get("module", ["signal"])
            if newActiveModule == "signal":
                newActiveModule = SIGNAL_MODULE
            elif newActiveModule == "mote":
                newActiveModule = MOTE_MODULE
            else:
                newActiveModule = NONE_MODULE

            updateActiveModule(newActiveModule)
            self.writeChunk("<p><strong>Active module updated!</strong></p>")

        self.serveHeader("module-config", {
                "%SIGNAL_CONV_MODULE%" : "checked" if newActiveModule == SIGNAL_MODULE else "",
                "%MOTE_MODULE%" : "checked" if newActiveModule == MOTE_MODULE else "",
                "%NONE_MODULE%" : "checked" if newActiveModule == NONE_MODULE else ""
                })
        self.serveFooter()

    def serveLeds(self, qs):
        global activeLeds

        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.send_header('Transfer-Encoding', 'chunked')
        self.end_headers()

        newActiveLeds = activeLeds

        if qs:
            for i in range(4):
                led = qs.get("led" + str(i), ["off"])
                newActiveLeds[i] = True if led[0] == 'on' else False

            updateActiveLeds(newActiveLeds)
            self.writeChunk("<p><strong>LEDs updated!</strong></p>")

        self.serveHeader("leds", {
                "%LED0%" : "checked" if activeLeds[0] else "",
                "%LED1%" : "checked" if activeLeds[1] else "",
                "%LED2%" : "checked" if activeLeds[2] else "",
                "%LED3%" : "checked" if activeLeds[3] else ""
                })
        self.serveFooter()

    def serveUpload(self, qs):
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.send_header('Transfer-Encoding', 'chunked')
        self.end_headers()

        self.serveHeader("upload")
        self.serveFooter()

    def do_POST(self):
#        global workingDirectory

#        print "cgi parse..."
        # Parse the form data posted
        form = cgi.FieldStorage(
            fp=self.rfile, 
            headers=self.headers,
            environ={'REQUEST_METHOD':'POST',
                     'CONTENT_TYPE':self.headers['Content-Type'],
                     })
#        print "cgi parse done..."

        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.send_header('Transfer-Encoding', 'chunked')
        self.end_headers()

#        self.serveHeader("upload-done")
#        cnt = self.rfile.read()
#        self.writeChunk(cnt)

        for field in form.keys():
            field_item = form[field]
            if field_item.filename:
                # The field contains an uploaded file
                file_data = field_item.file.read()
                file_len = len(file_data)
                break
#                self.writeChunk('\tUploaded %s (%d bytes)\n' % (field, 
#                                                                 file_len))
#            else:
#                # Regular form value
#                self.writeChunk('\t%s=%s\n' % (field, form[field].value))
#
#        self.serveFooter()
#       return

#            try:
#                file_data = open(filename, "r").read()
                
#                self.writeChunk(file_data)
#            except Exception, e:
#                print e

        if file_data:
            filename = "tmp-upload-file"
            with open(filename, "w") as outFile:
                outFile.write(file_data)
                outFile.close()

            closeAllSerial()

            arglist = ["python", "bsl.py", "--slow", "--invert-reset", "--invert-test", "-c", getComPortString(0), "-r", "-e", "-I", "-p", filename]
            try:
                retcode = subprocess.call(" ".join(arglist), shell=True)
            except OSError as e:
                sys.stderr.write("gcc execution failed: {}".format(str(e)))
                retcode = 1

            openAllSerial()

        if retcode == 0:
            self.serveHeader("upload-done")
        else:
            self.serveHeader("upload-error")

        self.serveFooter()
        return

    def do_GET(self):
        o = urlparse.urlparse(self.path)
        qs = urlparse.parse_qs(o.query)

        if o.path == "/":
            self.serveDefault()
        elif o.path == "/graphs":
            self.serveGraphs(qs)
        elif o.path == "/config":
            self.serveConfig(qs)
        elif o.path == "/upload":
            self.serveUpload(qs)
        elif o.path == "/module-config":
            self.serveModuleConfig(qs)
        elif o.path == "/leds":
            self.serveLeds(qs)
        else:
            self.send_response(404)
            self.send_header('Content-Type', 'text/html')
            self.send_header('Transfer-Encoding', 'chunked')
            self.end_headers()
            buffer = '<html><head><title>Not found!</title></head><body>\r\n'
            buffer += '<span style="font-family:Verdana; font-size:20px">NOT FOUND!!!</span>\r\n'
            self.writeChunk(buffer)
            self.writeFinalChunk()

def main():
    global defaultComPort

    try:
        with open("com-port.txt", "r") as f:
            s = f.read().strip()
            if s:
                defaultComPort = s
            else:
                defaultComPort = "COM13"
    except:
        defaultComPort = "COM13"

#    defaultComPort = "/dev/ttyUSB0"

    for i in range(TOTAL_MOTES):
        ports.append(SerialPort())

    try:
        openAllSerial()
        server = BaseHTTPServer.HTTPServer(('', HTTP_SERVER_PORT), HttpServerHandler)
        time.sleep(1)
        print "<http-server>: started"
        server.serve_forever()
    except Exception, e:
        print "<http-server>: exception occurred:", e
        return 1

if __name__ == '__main__':
    main()
