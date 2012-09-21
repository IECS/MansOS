#!/usr/bin/env python

import main, sys, os

testFileDir = 'tests'
architecture = 'testarch'
#architecture = 'telosb'
targetOS = 'mansos'
outputDirName = "build"
doCompile = False
compileArch = "telosb"

def runTest(sourceFileName):
    if not os.path.exists(outputDirName):
        os.makedirs(outputDirName)

    basename = os.path.basename(sourceFileName)

    outputFileName = outputDirName + '/' + basename[:-2] + 'c'

    if basename == "45-extras-cache.sl" or basename == "46-extras-cache-when.sl":
        arch = "schedtest"
    elif basename == "scen-sad.sl":
        arch = "sm3"
    else:
        arch = architecture

    sys.argv = ["./main.py", "-c", "-a", arch, "-t", targetOS, "-o", outputFileName, sourceFileName]

    try:
        ret = main.main()
    except Exception:
        print ("error compiling {}".format(sourceFileName))
        return

    if ret != 0: return

    # prepend output with the test script
    with open(outputFileName, 'r+') as outputFile:
        contents = outputFile.read()
        outputFile.seek(os.SEEK_SET, 0)
        outputFile.truncate()
        outputFile.write("/*\n")
        outputFile.write("# This .c file was generated from SEAL source:\n")
        with open(sourceFileName, 'r') as sourceFile:
            outputFile.write(sourceFile.read())
        outputFile.write("*/\n\n")
        outputFile.write(contents)

    if doCompile:
        os.system("cd build && make clean && make {}".format(compileArch))

def runTests():
    numTests = 0
    files = os.listdir(testFileDir)
    files.sort()
    for f in files:
        if f[-3:] != '.sl': continue
        sourceFileName = os.path.join(testFileDir, f)
        print ("\nprocessing " + sourceFileName + "...")
        runTest(sourceFileName)
        numTests += 1
        # break ###
    print ("{} tests successfully executed".format(numTests))

if __name__ == '__main__':
    runTests()
