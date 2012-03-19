#!/usr/bin/env python

import os, sys, getopt

#inputFileName = 'tests/91-comments.sl'
inputFileName = 'tests/00-use.sl'
outputFileName = 'main.c'
architecture = 'test'
#architecture = 'telosb'
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
    sys.stderr.write("Usage:\n");
    sys.stderr.write("  -a <arch>, --arch    Architecture\n");
    sys.stderr.write("  -o, --output [FILE]  Output to file (default: {0})\n".format(outputFileName));
    sys.stderr.write("  -V, --verbose        Verbose mode\n");
    sys.stderr.write("  -v, --version        Print version\n");
    sys.stderr.write("  -h, --help           Print this help\n");
    sys.exit(int(isError));

def parseCommandLine(argv):
    global inputFileName
    global outputFileName
    global architecture
    global verboseMode

    try:
        opts, args = getopt.getopt(sys.argv[1:], "a:ho:Vv", ["arch=", "help", "output=", "verbose", "version"])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        help(True)

    isError = False
    showHelp = False
    for o, a in opts:
        if o in ("-a", "--arch"):
            architecture = a
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
        else:
           print "o = ", o
           print "a = ", a

    if len(args):
        inputFileName = args[0]
        args = args[1:]
        if len(args):
            sys.stderr.write("Too many arguments given. ({0} remaining not parsed)\n".format(args));
            isError = True

    if showHelp or isError:
        help(isError)

def main():
    if not importsOk():
        exit(1)

    # import pathname where seal package is located
    sys.path.append('..')
    sys.path.append('../seal/components')
    from seal import parser, codegen, components

    parseCommandLine(sys.argv)
#    print "##################### cmd line processing done"
#    print "input=", inputFileName
#    print "output=", outputFileName
#    print "arch=", architecture
#    print "verbose=", verboseMode
#    exit(0)

    # read file to-be-parsed
    with open(inputFileName, 'r') as inputFile:
        contents = inputFile.read()
    if contents == None:
        print 'Failed to read file', inputFileName
        exit(1)

    # load available components
    components.componentRegister.load(architecture)
    # parse input file (SEAL code)
    parser = parser.SealParser(printLine, verboseMode)
    parser.run(contents)
    # generate C code to an output file
    if outputFileName is None:
        codegen.generate(sys.stdout)
    else:
        with open(outputFileName, 'w') as outputFile:
            codegen.generate(outputFile)

if __name__ == '__main__':
    main()
