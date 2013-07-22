#!/usr/bin/env python

# Write data

import os, sys, time
import urllib2

def main():
    host = "http://localhost:30001/write?port=/dev/ttyUSB0"
    try:
        req = urllib2.urlopen(host, "[data to write]")
        print("Reply data:")
        print(req.read())
    except Exception as e:
        print("exception occurred:")
        print(e)
        return 1

if __name__ == '__main__':
    main()
