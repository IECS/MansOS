#!/usr/bin/env python

#
# Remote access proxy: client-side BSL loader script
#

import os, sys, time
import urllib2
import ubsl

DEFAULT_PORT = 30001

def main():
    proxy = os.environ.get('BSLPROXY')
    if proxy is None:
        sys.stderr.write("BSLPROXY environment variable is not set!\n")
        sys.exit(1)

    # use only the first address, if more than one are given
    host = proxy.split(",")[0]
    try:
        portSet = int(host.split(":")[-1])
    except:
        # port not set, add default port to the URL
        host += ":" + str(DEFAULT_PORT)
    if host[:7] != "http://":
        host = "http://" + host

    sys.stdout.write("Using network BSL, connecting to host " + host + "\n")

    url = host + "/program?"

    # use UBSL to extract the name of the file
    cmdLine = ubsl.parseCommandLine()
    filename = cmdLine[0]
    port = cmdLine[2]

    argv = sys.argv
    if "python" in argv[0]: argv = argv[1:]
    argv[0] = "ubsl.py" # replace netbsl.py with ubsl.py
    joinedCommandLine = " ".join(argv)
    joinedCommandLine = urllib2.quote(joinedCommandLine)

    url += "args=" + joinedCommandLine + "&"
    url += "filename=" + urllib2.quote(filename) + "&"
    url += "port=" + urllib2.quote(port)

    contents = None
    with open(filename, "rb") as infile:
        contents = infile.read()

    if not bool(contents):
        sys.stderr.write("Input file not present, not readable or empty!\n")
        sys.exit(1)

    req = urllib2.urlopen(url, contents)
    if req.getcode() == 200:
        sys.stdout.write(req.read())
        sys.exit(0)
    else:
        sys.stderr.write("Request failed with HTTP return code " + str(req.getcode()) + "\n")
        sys.stderr.write(req.read())
        sys.exit(1)

if __name__ == '__main__':
    main()
