#
# MansOS web server - utilities
#

def isascii(c, printable = True):
    if 0x00 <= ord(c) <= 0x7f:
        if 0x20 <= ord(c) <= 0x7e:
            return True
        if printable and (c == '\r' or c == '\n' or c == '\t'):
            return True
        return False
    else:
        return False

def isasciiString(s):
    for c in s:
        if not isascii(c, False):
            return False
    return True
