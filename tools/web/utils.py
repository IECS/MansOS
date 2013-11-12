#
# MansOS web server - utility functions
#

# return true if a symbols is non-binary ASCII character
def isascii(c, printable = True):
    if 0x00 <= ord(c) <= 0x7f:
        if 0x20 <= ord(c) <= 0x7e:
            return True
        if printable and (c == '\r' or c == '\n' or c == '\t'):
            return True
        return False
    else:
        return False

# return true if all symbols are non-binary ASCII chars (ord(c) < 128)
def isasciiString(s):
    for c in s:
        if not isascii(c, False):
            return False
    return True

# convert string to /Titlecase/
def toTitleCase(s):
    if s == '': return ''
    return s[0].upper() + s[1:]

# convert string to /camelCase/
def toCamelCase(s):
    if s == '': return ''
    if len(s) > 1 and s[0].isupper() and s[1].isupper():
        # acronyms are unchanged
        return s
    return s[0].lower() + s[1:]

# read 4 bytes in little endian byteorder
def le32read(args):
    result = args[3] << 24
    result += args[2] << 16
    result += args[1] << 8
    result += args[0]
    return result

# write 4 bytes in little endian byteorder
def le32write(number):
    args = [0, 0, 0, 0]
    args[0] = number & 0xff
    args[1] = (number >> 8) & 0xff
    args[2] = (number >> 16) & 0xff
    args[3] = (number >> 24) & 0xff
    return args

def fileIsOver(f):
    currentPos = f.tell()
    f.seek(0, 2) # seek to end of file
    newPos = f.tell()
    f.seek(currentPos, 0)  # seek back to initial position
    return currentPos == newPos


def urlEscape(name):
    return name.replace("@", "_at_").replace(":", "_port_")

def urlUnescape(name):
    return name.replace("_at_", "@").replace("_port_", ":")
