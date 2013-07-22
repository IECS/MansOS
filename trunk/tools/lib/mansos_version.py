#
# Query the version of MansOS operating system
#

import os

def getMansosVersion(pathToMansos):
    result = ""
    try:
        with open(os.path.join(pathToMansos, "doc/VERSION")) as versionFile:
            result = versionFile.readline().strip()
    except Exception as e:
        print(e)
        return "?"
    return result
