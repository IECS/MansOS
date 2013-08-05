#!/usr/bin/env python

#
# Remote access proxy server - main file
#

from __future__ import print_function
import os, sys
import threading, time, cgi, signal, select, traceback, subprocess, urllib2
# add library directory to path
sys.path.append(os.path.join(os.getcwd(), '..', "lib"))
import daemon
import configuration
from motes import motes
import mansos_version

bslLock = threading.Lock()

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

def listenSerial():
    while True:
        for m in motes.getMotes():
            m.tryRead()
        # pause for a bit
        time.sleep(0.01)

def qsExtractBool(qs, name):
    value = False
    isSet = False
    if name in qs:
        arg = qs[name][0]
        try:
            value = bool(int(arg, 0))
            isSet = True
        except:
            try:
                value = bool(arg)
                isSet = True
            except:
                pass
    return (value, isSet)

def runSubprocess2(args):
    retcode = 0
    output = ""
    try:
#        print("runSubprocess2, args=" + args)
        proc = subprocess.Popen(args.split(), stderr = subprocess.STDOUT,
                                stdout = subprocess.PIPE, shell = False)
        (stdout, stderr) = proc.communicate()
        if stdout: output += stdout
        if stderr: output += stderr
        retcode = proc.returncode
    except OSError as e:
        print("runSubprocess2 OSError:" + str(e))
    except CalledProcessError as e:
        print("runSubprocess2 CalledProcessError:" + str(e))
        retcode = e.returncode
    except Exception as e:
        print("runSubprocess2 exception:" + str(e))
    finally:
        return (retcode, output)


class HttpServerHandler(BaseHTTPRequestHandler):
    server_version = 'MansOS/' + mansos_version.getMansosVersion(
        configuration.c.getCfgValue("mansosDirectory")) + ' Remote Access Server'
    protocol_version = 'HTTP/1.1'

    # overrides base class function, because in some versions
    # it tries to resolve dns and fails...
    def log_message(self, format, *args):
        sys.stderr.write("%s - - [%s] %s\n" %
                         (self.client_address[0],
                          self.log_date_time_string(),
                          format % args))

    def sendDefaultHeaders(self, content):
        self.send_header('Content-Type', 'text/html')
        # disable caching
        self.send_header('Cache-Control', 'no-store')
        # self.send_header('Connection', 'close')
        self.send_header('Content-Length', str(len(content)))

    def serveError(self, message):
        print("serveError: " + message)
        content = "Error: " + message
        self.send_response(500)
        self.sendDefaultHeaders(content)
        self.end_headers()
        self.wfile.write(content)

    def serve404Error(self, path, qs):
#        print("serve404")
        content = "Error: " + path + " not found on the server!"
        self.send_response(404)
        self.sendDefaultHeaders(content)
        self.end_headers()
        self.wfile.write(content)

    def serveDefault(self, qs):
#        print("serveDefault")
        self.send_response(200)
        self.sendDefaultHeaders("")
        self.end_headers()

    # Get list of all connected motes
    def servePorts(self, qs):
#        print("servePorts")
        content = ""
        all = motes.getMotes()
        for mote in all:
            content += mote.moteDescription.getCSVData() + "\n"
        self.send_response(200)
        self.sendDefaultHeaders(content)
        self.end_headers()
        self.wfile.write(content)

    # Get/set config of a specific port
    def serveControl(self, qs):
#        print("serveControl")

        # use "default" to set config for all ports without specific config
        if "port" not in qs:
            return self.serveError("port parameter expected!")
        port = qs.get("port")[0]

        dtr, doSetDtr = qsExtractBool(qs, "dtr")
        rts, doSetRts = qsExtractBool(qs, "rts")

        mote = motes.getMote(port)
        if mote is None:
            return self.serveError("Mote " + port + " not connected!")
        if not mote.ensureSerialIsOpen():
            return self.serveError("Port " + port + " cannot be opened!")

        content = ""
        if doSetDtr:
            content += "DTR = {}\n".format(dtr)
            mote.port.setDTR(dtr)
        if doSetRts:
            content += "RTS = {}\n".format(rts)
            mote.port.setRTS(rts)

        self.send_response(200)
        self.sendDefaultHeaders(content)
        self.end_headers()
        self.wfile.write(content)


    # Read a specific port
    def serveRead(self, qs):
#        print("serveRead")

        if "port" not in qs:
            return self.serveError("port parameter expected!")
        port = qs.get("port")[0]

        mote = motes.getMote(port)
        if mote is None:
            return self.serveError("Mote " + port + " not connected!")
        if not mote.ensureSerialIsOpen():
            return self.serveError("Port " + port + " cannot be opened!")

        content = mote.getData()

        self.send_response(200)
        self.sendDefaultHeaders(content)
        self.end_headers()
        self.wfile.write(content)

    # Write to a specific port
    def serveWrite(self, qs):
#        print("serveWrite")

        if "port" not in qs:
            return self.serveError("port parameter expected!")
        port = qs.get("port")[0]

        if self.headers.has_key('content-length'):
            length = int(self.headers['content-length'])
        else:
            return self.serveError("content-length parameter expected!")

        mote = motes.getMote(port)
        if mote is None:
            return self.serveError("Mote " + port + " not connected!")
        if not mote.ensureSerialIsOpen():
            return self.serveError("Port " + port + " cannot be opened!")

        toWrite = self.rfile.read(length)

        reply = mote.writeData(toWrite)
        content = str(reply)

        self.send_response(200)
        self.sendDefaultHeaders(content)
        self.end_headers()
        self.wfile.write(content)

    # Program an image
    def serveProgram(self, qs):
        global bslLock
#        print("serveProgram")

        if "filename" not in qs:
            return self.serveError("filename parameter expected!")
        if "args" not in qs:
            return self.serveError("args parameter expected!")
        if "port" not in qs:
            return self.serveError("port parameter expected!")
        filename = urllib2.unquote(qs.get("filename")[0])

        if self.headers.has_key('content-length'):
            length = int(self.headers['content-length'])
        else:
            return self.serveError("content-length parameter expected!")
        arguments = urllib2.unquote(qs.get("args")[0])
        port = urllib2.unquote(qs.get("port")[0])

        mote = motes.getMote(port)
        if mote is None:
            return self.serveError("Mote " + port + " not connected!")
        mote.ensureSerialIsClosed() # Note: will not reopen automatically!

        dirname = os.path.dirname(os.path.join("tmpdir", filename))
        if not os.path.exists(dirname):
            os.makedirs(dirname)
        with open(os.path.join("tmpdir", filename), "wb") as tmpfile:
            tmpfile.write(self.rfile.read(length))

        bslDir = os.path.abspath(os.path.join(configuration.c.getCfgValue("mansosDirectory"),
                                              "mos", "make", "scripts"))
        if configuration.c.getCfgValueAsBool("slowUpload"):
            arguments += " --slow"
        arguments = "python " + bslDir + os.path.sep + arguments

        with bslLock:
            os.chdir("tmpdir")
            (retcode, content) = runSubprocess2(arguments)
            os.chdir("..")

        self.send_response(200)
        self.sendDefaultHeaders(content)
        self.end_headers()
        self.wfile.write(content)


    def do_GET(self):
        o = urlparse(self.path)
        qs = parse_qs(o.query)

        if o.path == "/" or o.path == "/default":
            self.serveDefault(qs)
        elif o.path == "/ports":
            self.servePorts(qs)
        elif o.path == "/control":
            self.serveControl(qs)
        elif o.path == "/read":
            self.serveRead(qs)
        else:
            self.serve404Error(o.path, qs)


    def do_POST(self):
        o = urlparse(self.path)
        qs = parse_qs(o.query)

        if o.path == "/write":
            self.serveWrite(qs)
        elif o.path == "/program":
            self.serveProgram(qs)
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


def main():
    try:
        if configuration.c.getCfgValueAsBool("createDaemon"):
            # detach from controlling tty and go to background
            daemon.createDaemon()
        # start the server
        port = configuration.c.getCfgValueAsInt("port")
        server = ThreadingHTTPServer(('', port), HttpServerHandler)
        # load motes
        motes.addAll()
        # start listening thread
        listenThread = threading.Thread(target = listenSerial)
        listenThread.start()
        # report ok and enter the main loop
        print("<remoteaccess>: started, listening to TCP port {}, serial baudrate {}".format(
                port, configuration.c.getCfgValueAsInt("baudrate")))
        server.serve_forever()
    except Exception as e:
        print("<remoteaccess>: exception occurred:")
        print(e)
        print(traceback.format_exc())
        return 1


if __name__ == '__main__':
    main()
