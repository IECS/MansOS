#!/usr/bin/env python

import os, sys, getopt

#inputFileName = 'tests/91-comments.sl'
inputFileName = 'tests/00-use.sl'
outputFileName = 'main.c'
#architecture = 'test'
architecture = 'telosb'
targetOS = 'mansos'
verboseMode = False

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
    print line

def help(isError):
    sys.stderr.write("Usage:\n")
    sys.stderr.write("  -a <arch>, --arch     Target architecture (e.g. 'telosb')\n")
    sys.stderr.write("  -t <target>, --target Target OS (e.g. 'mansos')\n")
    sys.stderr.write("  -o, --output [FILE]   Output to file (default: {0})\n".format(outputFileName))
    sys.stderr.write("  -V, --verbose         Verbose mode\n")
    sys.stderr.write("  -v, --version         Print version\n")
    sys.stderr.write("  -h, --help            Print this help\n")
    sys.exit(int(isError))

def parseCommandLine(argv):
    global inputFileName
    global outputFileName
    global architecture
    global verboseMode

    try:
        opts, args = getopt.getopt(sys.argv[1:], "a:ho:t:Vv", ["arch=", "help", "output=", "target=" "verbose", "version"])
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
    sys.path.append('..')
    sys.path.append('../seal/components')
    from seal import parser, generator, components

    parseCommandLine(sys.argv)

    # read file to-be-parsed
    with open(inputFileName, 'r') as inputFile:
        contents = inputFile.read()
    if contents == None:
        sys.stderr.write('Failed to read file {0}'.format(inputFileName))
        exit(1)

    # load available components
    components.componentRegister.load(architecture)
    # parse input file (SEAL code)
    parser = parser.SealParser(printLine, verboseMode)
    parser.run(contents)

    # print parser.result.getCode(0)
    parser.result.addComponents()

    # generate C code to an output file
    g = generator.createGenerator(targetOS)
    if g is None:
        sys.stderr.write('Failed to find code generator for target OS {0}'.format(targetOS))
        exit(1)

    if outputFileName is None:
        g.generate(sys.stdout)
    else:
        outputDirName = os.path.dirname(outputFileName)
        if len(outputDirName): outputDirName += '/'

        if not os.path.exists(outputDirName):
            os.makedirs(outputDirName)
        with open(outputFileName, 'w') as outputFile:
            g.generate(outputFile)
        with open(outputDirName + "Makefile", 'w') as outputFile:
            g.generateMakefile(outputFile, outputFileName)
        with open(outputDirName + "config", 'w') as outputFile:
            g.generateConfigFile(outputFile)

if __name__ == '__main__':
    main()
