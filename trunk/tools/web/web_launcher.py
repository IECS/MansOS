#!/usr/bin/env python

#
# MansOS web server - launcher (launches server and default browser)
#

import mansos_server
import time
import webbrowser
import os
import subprocess
import string
import threading

DEFAULT_PORT = 30000

serverStarted = False

def launchBrowser():
    # wait for server to start starting...
    while not serverStarted:
        time.sleep(0.001)

    # wait for server to finish starting
    time.sleep(1.0)

    # search for port in server's config file
    serverPort = DEFAULT_PORT
    with open("server.cfg", 'r') as f:
        contents = f.read()
        position = string.find(contents, "port=")
        if position != -1:
            position += 5
            portStr = contents[position:].split()[0]
            try:
                serverPort = int(portStr)
            except:
                serverPort = DEFAULT_PORT

    url = "http://localhost:" + str(serverPort)
    if os.name == "posix":
        controller = webbrowser.get('Firefox')
    else:
        controller = webbrowser.get('windows-default')
    controller.open_new_tab(url)


def isProcessRunning(names):
    myPid = os.getpid()
    output = subprocess.check_output(["ps", "x"])
    for line in output.splitlines():
        for name in names:
            if string.find(line, name) == -1: continue
            try:
                pid = int(line.split()[0])
                if pid == myPid: continue
            except:
                continue
            return True
    return False


def main():
    global serverStarted

    try:
        browserThread = threading.Thread(target=launchBrowser)
        browserThread.start()

        serverExists = isProcessRunning(['mansos_server', 'web_launcher'])

        serverStarted = True
        if not serverExists:
            mansos_server.main()
    except Exception as e:
        raise

if __name__ == '__main__':
    main()
