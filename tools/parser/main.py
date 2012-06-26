#!/usr/bin/python
# - because /usr/bin/env python does not works when called from IDE on Windows

import os, sys, getopt

inputFileName = 'test.sl'
outputFileName = 'main.c'
architecture = 'testarch'
#architecture = 'telosb'
targetOS = 'mansos'
pathToOS = '..'
verboseMode = False
testMode = False

def exitProgram(code):
    if not testMode:
        exit(code)
    print "Would exit from program with code " + str(code)
    raise Exception

def importsOk():
    plyModuleOK = True     # Python Lex Yacc - for compilation
    
    try:
        import ply
    except ImportError:
        plyModuleOK = False

    if not plyModuleOK:
        if os.name == 'posix':
            installStr = "Make sure you have installed required modules. Run:\n\tapt-get install"
        else: 
            installStr = "Make sure you have installed modules:"
            
        print "Cannot run SEAL parser:"

        if not plyModuleOK:
            print "\tPLY module not found"
            installStr += " python-ply"

        print installStr
        return False
    return True

def printLine(line):
    sys.stderr.write(line)

def help(isError):
    sys.stderr.write("Usage:\n")
    sys.stderr.write("  -a <arch>, --arch     Target architecture (e.g. 'telosb')\n")
    sys.stderr.write("  -t <target>, --target Target OS (e.g. 'mansos')\n")
    sys.stderr.write("  -o, --output <file>   Output to file (default: {0}, - for stdout)\n".format(outputFileName))
    sys.stderr.write("  -p, --path <path>     PAth to the target OS folder (default: {0})\n".format(pathToOS))
    sys.stderr.write("  -V, --verbose         Verbose mode\n")
    sys.stderr.write("  -v, --version         Print version\n")
    sys.stderr.write("  -e, --errors          Test mode (ignore errors)\n")
    sys.stderr.write("  -h, --help            Print this help\n")
    sys.exit(int(isError))

def parseCommandLine(argv):
    global inputFileName
    global outputFileName
    global architecture
    global verboseMode
    global testMode
    global pathToOS

    try:
        opts, args = getopt.getopt(sys.argv[1:], "a:eho:p:t:Vv",
                                   ["arch=", "errors", "help", "output=", "path=", "target=" "verbose", "version"])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        help(True)

    isError = False
    showHelp = False
    for o, a in opts:
        if o in ("-a", "--arch"):
            architecture = a.lower()
        if o in ("-t", "--target"):
            targetOS = a.lower()
        elif o in ("-v", "--version"):
            from seal import mansos
            print "MansOS version:", mansos.VERSION, "(Release date: " + mansos.RELEASE_DATE + ")"
            sys.exit(0)
        elif o in ("-V", "--verbose"):
            verboseMode = True
        elif o in ("-h", "--help"):
            showHelp = True
        elif o in ("-o", "--output"):
            outputFileName = a
        elif o in ("-p", "--path"):
            pathToOS = a
        elif o in ("-e", "--errors"):
            testMode = True

    if len(args):
        inputFileName = args[0]
        args = args[1:]
        if len(args):
            sys.stderr.write("Too many arguments given. ({0} remaining not parsed)\n".format(args))
            isError = True

    if showHelp or isError:
        help(isError)

def main():
    if not importsOk():
        exit(1)

    # import pathname where seal package is located
    dirname = os.path.dirname(os.path.realpath(__file__))
    sys.path.append(dirname + '/..')
    sys.path.append(dirname + '/../seal/components')
    from seal import generator
    # in case tis is used multiple times
    generator.components.clearGlobals()

    parseCommandLine(sys.argv)

    # read file to-be-parsed
    with open(inputFileName, 'r') as inputFile:
        contents = inputFile.read()
    if contents == None:
        sys.stderr.write('Failed to read file {0}'.format(inputFileName))
        exitProgram(1)

    # parse input file (SEAL code)
    parser = generator.SealParser(architecture, printLine, verboseMode)
    parser.run(contents)
    if parser.isError:
        exitProgram(1) # do not generate output file in this case

    # generate C code to an output file
    g = generator.createGenerator(targetOS)
    if g is None:
        sys.stderr.write('Failed to find code generator for target OS {0}'.format(targetOS))
        exitProgram(1)

    if outputFileName == '-':
        g.generate(sys.stdout)
    else:
        outputDirName = os.path.dirname(outputFileName)
        if len(outputDirName):
            outputDirName += '/'
            if not os.path.exists(outputDirName):
                os.makedirs(outputDirName)
        with open(outputFileName, 'w') as outputFile:
            g.generate(outputFile)
        with open(outputDirName + "Makefile", 'w') as outputFile:
            g.generateMakefile(outputFile, outputFileName, pathToOS)
        with open(outputDirName + "config", 'w') as outputFile:
            g.generateConfigFile(outputFile)
        if parser.isError:
            # cleanup
            os.remove(outputFileName)
            os.remove(outputDirName + "Makefile")
            os.remove(outputDirName + "config")

if __name__ == '__main__':
    main()
