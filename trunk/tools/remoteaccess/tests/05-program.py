#!/usr/bin/env python

# program a mote (pseudotest, not working)

import os, sys, time
import urllib2

def main():
    host = "http://localhost:30001/program?port=/dev/ttyUSB0&args=a&filename=file.ihex"
    try:
        req = urllib2.urlopen(host, "[program data]")
        print("Reply data:")
        print(req.read())
    except Exception as e:
        print("exception occurred:")
        print(e)
        return 1

if __name__ == '__main__':
    main()
