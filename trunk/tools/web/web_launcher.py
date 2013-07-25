#!/usr/bin/env python

#
# MansOS web server - launcher (launches server and default browser)
#

from __future__ import print_function
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

    # open the web browser (Firefox in Linux, user's preferred on Windows)
    if os.name == "posix":
        # will return default system's browser if Firefox is not installed
        controller = webbrowser.get('Firefox')
    else:
        controller = webbrowser.get('windows-default')

    url = "http://localhost"
    if serverPort != 80: url += ":" + str(serverPort)
    controller.open_new_tab(url)


def isProcessRunningPosix(names):
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


def isProcessRunningWindows(names):
    # Finding process info on Windows (all versions, including e.g. WinXP Home)
    # is too complicated without Python extension packages :(
    return False


def isProcessRunning(names):
    if os.name == "posix":
        return isProcessRunningPosix(names)
    else:
        return isProcessRunningWindows(names)


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
