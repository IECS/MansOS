#!/usr/bin/env python

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


def main():
    if not importsOk():
        exit(1)
    
    # Go to real directory for import to work
    import os
    import sys

    sys.path.append('..')
#    os.chdir("..")
#    os.chdir("/home/atis/work/mansos/tools")
#    print os.path.dirname(os.path.realpath(__file__))
#    os.chdir(os.path.dirname(os.path.realpath(__file__)))
    from seal import sealParser_yacc

    s = "use Foobar, param1 1000, param2 1s, param3 1500ms;"
    parser = sealParser_yacc.SealParser(printLine)
    parser.run(s)


if __name__ == '__main__':
    main()
