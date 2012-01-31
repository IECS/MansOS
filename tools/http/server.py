#!/usr/bin/env python
#
#
# Copyright (c) 2010-2012 the MansOS team. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of  conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#
# URLs:
#  mote?id=x - read data from mote #x
#  allmotes?start=x&end=y - read data from motes #x to #y
#
# To get plain (undecorated) text output append "raw=1" to query string
#

import os, urlparse, BaseHTTPServer, threading, time, serial, select, socket

HTTP_SERVER_PORT = 38401
HTTP_SERVER_NOTIFICATON_PORT = 38402

TOTAL_MOTES = 7
BAUDRATE = 38400

class SerialPort:
    number = 0
    handle = None
    buffer = ""

ports = []

def openSerial(motenr, port):
    port.number = motenr
    try:
        port.handle = serial.Serial("/dev/ttyUSB%d" % motenr, BAUDRATE, timeout=1,
                                    parity=serial.PARITY_NONE, rtscts=1)
    except Exception, e:
        print "\nSerial exception:\n\t", e
        return
    
    print "Listening to serial port: ", port.handle.portstr, ", rate: ", BAUDRATE


def listenSerial():
    global ports
    for i in range(len(ports)):
        openSerial(i, ports[i])

    udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udpsock.bind(('', HTTP_SERVER_NOTIFICATON_PORT))

    while True:
        readfd = []
        errfd = []

        for p in ports:
            # save memory... (XXX: this should be dependent on whether the port has some readers...)
            if len(p.buffer) > 256:
                p.buffer = ""
            if p.handle != None:
                readfd.append(p.handle)
                errfd.append(p.handle)
        readfd.append(udpsock)
        errfd.append(udpsock)

        (readfd, _, errfd) = select.select(readfd, [], errfd, 60.0)

        for fd in readfd:
            if fd == udpsock:
                # got a signal
                buf = fd.recv(3)
                motenr = int(buf[1:])
                if buf[0] == '-':
                    print 'temporary closing', motenr
                    if ports[motenr].handle != None:
                        ports[motenr].handle.close()
                        ports[motenr].handle = None
                elif buf[0] == '+':
                    print 'reopening', motenr
                    if ports[motenr].handle == None:
                        openSerial(motenr, ports[motenr])
                break

            buf = fd.read(1)
            for p in ports:
                if p.handle == fd:
                    p.buffer += buf
                    break

        for fd in errfd:
            for p in ports:
                if p.handle == fd:
                    print "<http-server>: closing a serial port, there was an error..."
                    fd.close()
                    p.handle = None


class HttpServerHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    server_version = 'TestBedAccess Web Server/0.1'
    protocol_version = 'HTTP/1.1' # 'HTTP/1.0' is the default, but we want chunked encoding

    def writeChunk(self, buffer):
        if self.wfile == None: return
        if self.wfile._sock == None: return
        self.wfile.write("%x\r\n" % len(buffer));
        self.wfile.write(buffer)
        self.wfile.write("\r\n");

    def serveOneMoteIter(self, motenr, isRaw):
        if self.wfile == None: return
        if self.wfile._sock == None: return
        buffer = ""
        p = ports[motenr]
        if len(p.buffer) > 0:
            lines = p.buffer.split('\n')
            p.buffer = ""
            for l in lines:
                l = l.strip()
                if l == "": continue
                if isRaw:
                    buffer += l
                else:
                    buffer += '<a href="/mote?id=%d"><span style="color:#BB4B00; text-decoration:none">&lt;mote %d&gt;: </span></a>' % (motenr, motenr)
                    buffer += l
                    buffer += "<br/>"
                buffer += "\r\n"
	if len(buffer) > 0: self.writeChunk(buffer)
        threading.Timer(1.0, self.serveOneMoteIter, args=([motenr, isRaw])).start()


    def serveAllMotesIter(self, start, end, isRaw):
        if self.wfile == None: return
        if self.wfile._sock == None: return
        buffer = ""
        for motenr in range(start, end + 1):
            p = ports[motenr]
            if len(p.buffer) <= 0: continue
            lines = p.buffer.split('\n')
            p.buffer = ""
            for l in lines:
                l = l.strip()
                if l == "": continue
                if isRaw:
                    buffer += l
                else:
                    buffer += '<a href="/mote?id=%d"><span style="color:#BB4B00; text-decoration:none">&lt;mote %d&gt;: </span></a>' % (motenr, motenr)
                    buffer += l
                    buffer += "<br/>"
                buffer += "\r\n"
	if len(buffer) > 0: self.writeChunk(buffer)
        threading.Timer(1.0, self.serveAllMotesIter, args=([start, end, isRaw])).start()

    def serveOneMote(self, qs):
        if "id" not in qs:
            print "serveOneMote: 'id' attribute not present in query string"
            self.send_response(400)
            self.end_headers()
            return
        motenr = int(qs["id"][0])
        if motenr >= TOTAL_MOTES:
            print "serveOneMote: mote index out of range:", motenr
            self.send_response(400)
            self.end_headers()
            return
        isRaw = "raw" in qs;
        self.close_connection = False
        self.send_response(200)
        self.send_header('Content-Type', 'text/plain' if isRaw else 'text/html')
        self.send_header('Transfer-Encoding', 'chunked')
        self.end_headers()
        if not isRaw:
            buffer = "<html><head><title>Mote #%d</title></head><body>\r\n" % motenr
            buffer += '<span style="font-family:Verdana; font-size:12px">\r\n'
            buffer += 'This is serial port output for mote #%d. To see output for all motes go ' % motenr
            buffer += '<a href="/allmotes?start=0&end=%d">here</a>.<br/>\r\n' % (TOTAL_MOTES - 1)
            self.writeChunk(buffer)
        self.serveOneMoteIter(motenr, isRaw)

    def serveAllMotes(self, qs):
        if ("start" not in qs) or ("end" not in qs):
            print "serveAllMotes: 'start' and/or 'end' attributes not present in query string"
            self.send_response(400)
            self.end_headers()
            return
        start = int(qs["start"][0])
        end = int(qs["end"][0])
        if start > end:
            tmp = start
            start = end
            end = tmp
        if end >= TOTAL_MOTES:
            print "serveAllMotes: mote index out of range:", motenr
            self.send_response(400)
            self.end_headers()
            return
        isRaw = "raw" in qs;
        self.close_connection = False
        self.send_response(200)
        self.send_header('Content-Type', 'text/plain' if isRaw else 'text/html')
        self.send_header('Transfer-Encoding', 'chunked')
        self.end_headers()
        if not isRaw:
            buffer = "<html><head><title>Motes #%d - #%d</title></head><body>\r\n" % (start, end);
            buffer += '<span style="font-family:Verdana; font-size:12px">'
            self.writeChunk(buffer)
        self.serveAllMotesIter(start, end, isRaw)

    def serveDefault(self):
        start = 0
        end = TOTAL_MOTES - 1
        self.close_connection = False
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.send_header('Transfer-Encoding', 'chunked')
        self.end_headers()
        buffer = "<html><head><title>Motes #%d - #%d</title></head><body>\r\n" % (start, end);
        buffer += '<span style="font-family:Verdana; font-size:12px">'
        self.writeChunk(buffer)
        self.serveAllMotesIter(start, end, False)


    def do_GET(self):
        o = urlparse.urlparse(self.path)
        qs = urlparse.parse_qs(o.query)

        if o.path == "/":
            self.serveDefault()
        elif o.path == "/mote":
            self.serveOneMote(qs)
        elif o.path == "/allmotes":
            self.serveAllMotes(qs)
        else:
            self.send_response(404)
            self.send_header('Content-Type', 'text/html')
            self.end_headers()
            self.wfile.write('<html><head><title>Not found!</title></head><body>\r\n');
	    self.wfile.write('<span style="font-family:Verdana; font-size:20px">NOT FOUND!!!</span>\r\n')

# 
# createDaemon() function
# Copyright (C) 2005 Chad J. Schroeder
#
def createDaemon():
   """Detach a process from the controlling terminal and run it in the
   background as a daemon.
   """
   UMASK = 0
   WORKDIR = "/"
   MAXFD = 1024
   if (hasattr(os, "devnull")):
       REDIRECT_TO = os.devnull
   else:
       REDIRECT_TO = "/dev/null"

   try:
      # Fork a child process so the parent can exit.  This returns control to
      # the command-line or shell.  It also guarantees that the child will not
      # be a process group leader, since the child receives a new process ID
      # and inherits the parent's process group ID.  This step is required
      # to insure that the next call to os.setsid is successful.
      pid = os.fork()
   except OSError, e:
      raise Exception, "%s [%d]" % (e.strerror, e.errno)

   if (pid == 0):	# The first child.
      # To become the session leader of this new session and the process group
      # leader of the new process group, we call os.setsid().  The process is
      # also guaranteed not to have a controlling terminal.
      os.setsid()

      # Is ignoring SIGHUP necessary?
      #
      # It's often suggested that the SIGHUP signal should be ignored before
      # the second fork to avoid premature termination of the process.  The
      # reason is that when the first child terminates, all processes, e.g.
      # the second child, in the orphaned group will be sent a SIGHUP.
      #
      # "However, as part of the session management system, there are exactly
      # two cases where SIGHUP is sent on the death of a process:
      #
      #   1) When the process that dies is the session leader of a session that
      #      is attached to a terminal device, SIGHUP is sent to all processes
      #      in the foreground process group of that terminal device.
      #   2) When the death of a process causes a process group to become
      #      orphaned, and one or more processes in the orphaned group are
      #      stopped, then SIGHUP and SIGCONT are sent to all members of the
      #      orphaned group." [2]
      #
      # The first case can be ignored since the child is guaranteed not to have
      # a controlling terminal.  The second case isn't so easy to dismiss.
      # The process group is orphaned when the first child terminates and
      # POSIX.1 requires that every STOPPED process in an orphaned process
      # group be sent a SIGHUP signal followed by a SIGCONT signal.  Since the
      # second child is not STOPPED though, we can safely forego ignoring the
      # SIGHUP signal.  In any case, there are no ill-effects if it is ignored.
      #
      # import signal           # Set handlers for asynchronous events.
      # signal.signal(signal.SIGHUP, signal.SIG_IGN)

      try:
         # Fork a second child and exit immediately to prevent zombies.  This
         # causes the second child process to be orphaned, making the init
         # process responsible for its cleanup.  And, since the first child is
         # a session leader without a controlling terminal, it's possible for
         # it to acquire one by opening a terminal in the future (System V-
         # based systems).  This second fork guarantees that the child is no
         # longer a session leader, preventing the daemon from ever acquiring
         # a controlling terminal.
         pid = os.fork()	# Fork a second child.
      except OSError, e:
         raise Exception, "%s [%d]" % (e.strerror, e.errno)

      if (pid == 0):	# The second child.
         # Since the current working directory may be a mounted filesystem, we
         # avoid the issue of not being able to unmount the filesystem at
         # shutdown time by changing it to the root directory.
         os.chdir(WORKDIR)
         # We probably don't want the file mode creation mask inherited from
         # the parent, so we give the child complete control over permissions.
         os.umask(UMASK)
      else:
         # exit() or _exit()?  See below.
         os._exit(0)	# Exit parent (the first child) of the second child.
   else:
      # exit() or _exit()?
      # _exit is like exit(), but it doesn't call any functions registered
      # with atexit (and on_exit) or any registered signal handlers.  It also
      # closes any open file descriptors.  Using exit() may cause all stdio
      # streams to be flushed twice and any temporary files may be unexpectedly
      # removed.  It's therefore recommended that child branches of a fork()
      # and the parent branch(es) of a daemon use _exit().
      os._exit(0)	# Exit parent of the first child.

   # Close all open file descriptors.  This prevents the child from keeping
   # open any file descriptors inherited from the parent.  There is a variety
   # of methods to accomplish this task.  Three are listed below.
   #
   # Try the system configuration variable, SC_OPEN_MAX, to obtain the maximum
   # number of open file descriptors to close.  If it doesn't exists, use
   # the default value (configurable).
   #
   # try:
   #    maxfd = os.sysconf("SC_OPEN_MAX")
   # except (AttributeError, ValueError):
   #    maxfd = MAXFD
   #
   # OR
   #
   # if (os.sysconf_names.has_key("SC_OPEN_MAX")):
   #    maxfd = os.sysconf("SC_OPEN_MAX")
   # else:
   #    maxfd = MAXFD
   #
   # OR
   #
   # Use the getrlimit method to retrieve the maximum file descriptor number
   # that can be opened by this process.  If there is not limit on the
   # resource, use the default value.
   #
   import resource		# Resource usage information.
   maxfd = resource.getrlimit(resource.RLIMIT_NOFILE)[1]
   if (maxfd == resource.RLIM_INFINITY):
      maxfd = MAXFD
  
   # Iterate through and close all file descriptors.
   for fd in range(0, maxfd):
      try:
         os.close(fd)
      except OSError:	# ERROR, fd wasn't open to begin with (ignored)
         pass

   # Redirect the standard I/O file descriptors to the specified file.  Since
   # the daemon has no controlling terminal, most daemons redirect stdin,
   # stdout, and stderr to /dev/null.  This is done to prevent side-effects
   # from reads and writes to the standard I/O file descriptors.

   # This call to open is guaranteed to return the lowest file descriptor,
   # which will be 0 (stdin), since it was closed above.
   os.open(REDIRECT_TO, os.O_RDWR)	# standard input (0)

   # Duplicate standard input to standard output and standard error.
   os.dup2(0, 1)			# standard output (1)
   os.dup2(0, 2)			# standard error (2)

   return(0)

def main():
    for i in range(TOTAL_MOTES):
        ports.append(SerialPort())

    try:
        createDaemon()
        threading.Thread(target=listenSerial).start()
        server = BaseHTTPServer.HTTPServer(('', HTTP_SERVER_PORT), HttpServerHandler)
        time.sleep(1)
        print "<http-server>: started"
        server.serve_forever()
    except Exception, e:
        print "<http-server>: exception occurred:", e

if __name__ == '__main__':
    main()
