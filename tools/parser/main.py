#!/usr/bin/python
# (because /usr/bin/env python does not work when called from IDE on Windows)

#
# Copyright (c) 2012 Atis Elsts
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of  conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os, sys, getopt, shutil

inputFileName = 'test.sl'
outputFileName = 'main.c'
architecture = 'testarch'
#architecture = 'msp430'
#architecture = 'pc'
targetOS = 'mansos'
pathToOS = '../..'
verboseMode = False
testMode = False

def exitProgram(code):
    if not testMode:
        exit(code)
    print ("Would exit from program with code " + str(code))
    raise Exception

def importsOk():
    plyModuleOK = True     # Python Lex Yacc - for compilation
    
    try:
        import ply
    except ImportError:
        plyModuleOK = False

    if not plyModuleOK:
        if os.name == 'posix':
            installStr = "Make sure you have installed required modules. Run:\n\tsudo apt-get install"
        else: 
            installStr = "Make sure you have installed modules:"
            
        print ("Cannot run SEAL parser:")

        if not plyModuleOK:
            print ("\tPLY module not found")
            installStr += " python-ply"

        print (installStr)
        return False
    return True

def printLine(line):
    sys.stderr.write(line)

def help(isError):
    sys.stderr.write("Usage:\n")
    sys.stderr.write("  -a <arch>, --arch     Target architecture (defalt: {})\n".format(architecture))
    sys.stderr.write("  -t <target>, --target Target OS (default: {0})\n".format(targetOS))
    sys.stderr.write("  -o, --output <file>   Output to file, '-' for stdout (default: {0})\n".format(outputFileName))
    sys.stderr.write("  -p, --path <path>     Path to the target OS installation (default: {0})\n".format(pathToOS))
    sys.stderr.write("  -V, --verbose         Verbose mode\n")
    sys.stderr.write("  -v, --version         Print version and exit\n")
    sys.stderr.write("  -c, --continue        Continue on errors (test mode)\n")
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
        opts, args = getopt.getopt(sys.argv[1:], "a:cho:p:t:Vv",
                   ["arch=", "continue", "help", "output=",
                    "path=", "target=", "verbose", "version"])
    except getopt.GetoptError as err:
        # print help information and exit:
        print (str(err)) # will print something like "option -a not recognized"
        help(True)

    isError = False
    showHelp = False
    for o, a in opts:
        if o in ("-a", "--arch"):
            architecture = a.lower()
        if o in ("-t", "--target"):
            targetOS = a.lower()
        elif o in ("-v", "--version"):
            versionFile = os.path.join("../..", "doc/VERSION")
            release = "Unknown"
            date = "Unknown"
            try:
                f = open(versionFile, "r")
                lines = f.readlines()
                f.close()
                if len(lines) > 0:
                    release = lines[0].strip()
                if len(lines) > 1:
                    date = lines[1].strip()
            except:
                pass
            print ("MansOS version: " + release + " (Release date: " + date + ")")
            sys.exit(0)
        elif o in ("-V", "--verbose"):
            verboseMode = True
        elif o in ("-h", "--help"):
            showHelp = True
        elif o in ("-o", "--output"):
            outputFileName = a
        elif o in ("-p", "--path"):
            pathToOS = a
        elif o in ("-c", "--continue"):
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
    selfDirname = os.path.dirname(os.path.realpath(__file__))
    sys.path.append(os.path.join(selfDirname, pathToOS, 'tools'))
    sys.path.append(os.path.join(selfDirname, pathToOS, 'tools', 'seal', 'components'))

    from seal import generator
    # in case this is used multiple times
    generator.components.clearGlobals()

    parseCommandLine(sys.argv)

    # for extension modules
    sys.path.append(os.path.join(os.getcwd(), os.path.dirname(inputFileName)))

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
            outputDirName += os.sep
            if not os.path.exists(outputDirName):
                os.makedirs(outputDirName)

        numDirs = len(os.path.normpath(outputFileName).split(os.sep)) - 1
        dirname = os.path.dirname(os.path.realpath(outputFileName))
        if os.path.isabs(pathToOS):
            makefilePathToOS = pathToOS.strip('\\'); # \ is special character, creates problems in makefile where this path is inserted
        else:
            makefilePathToOS = os.path.normpath(dirname + os.sep + ('/..' * numDirs) + os.sep + pathToOS)

        with open(outputFileName, 'w') as outputFile:
            g.generate(outputFile)
        with open(outputDirName + "Makefile", 'w') as outputFile:
            g.generateMakefile(outputFile, outputFileName, makefilePathToOS)

        # use SEAL application's config file as the basis
        try:
            shutil.copyfile(outputDirName + ".." + os.sep + "config", outputDirName + "config-tmp")
        except IOError as e:
            try:
                os.remove(outputDirName + "config-tmp")
            except OSError as e:
                pass

        with open(outputDirName + "config-tmp", 'a+') as outputFile:
            g.generateConfigFile(outputFile)

        # replace the config file only if different: saves rebuiding time.
        try:
            isSame = (os.system("cmp -s " + outputDirName + "config-tmp " + outputDirName + "config") == 0)
        except:
            isSame = False
        if not isSame:
            try:
                shutil.move(outputDirName + "config-tmp", outputDirName + "config")
            except Exception as ex:
                print (ex)

        if generator.components.componentRegister.isError:
            # cleanup
            os.remove(outputFileName)
            os.remove(outputDirName + "Makefile")
            os.remove(outputDirName + "config")
            return -1

        if g.isComponentUsed("network"):
            g.generateBaseStationCode(os.path.join(outputDirName, 'bs'), makefilePathToOS)
            g.generateForwarderCode(os.path.join(outputDirName, 'fwd'), makefilePathToOS)
            g.generateCollectorCode(os.path.join(outputDirName, 'coll'), makefilePathToOS)
        elif g.isComponentUsed("radio"):
            g.generateBaseStationCode(os.path.join(outputDirName, 'bs'), makefilePathToOS)

        if g.isComponentUsed("sdcard"):
            g.generateRaw2Csv(outputDirName, os.path.join(selfDirname, 'raw2csv-template.py'))

    return 0

if __name__ == '__main__':
    exit(main())
