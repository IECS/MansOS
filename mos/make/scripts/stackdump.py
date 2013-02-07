#!/usr/bin/python

#
# Copyright (c) 2013 the MansOS team. All rights reserved.
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

import sys, subprocess, string, re, os

#########################################

class Regexp:
    def __init__(self, pattern):
        self.pattern = re.compile(pattern, 0)
        self.hit = None
    def match(self, line):
        self.hit = re.match(self.pattern, line)
        return self.hit
    def search(self, line):
        self.hit = re.search(self.pattern, line)
        return self.hit
    def group(self, idx):
        return self.hit.group(idx)

#########################################

class Function:
    def __init__(self, name, address):
        # symbolic name of this function
        self.name = name
        # address of this function
        self.address = address
        # the (worst-case) stack usage trace in this function
        self.trace = []
        # the functions this function calls
        self.callees = set()
        # memoization
        self.calculatedStackUsage = None

    def addOp(self, op, value = 0, callee = None):
        if op == "pop":
            self.trace.append((+2, None))
        elif op == "push":
            self.trace.append((-2, None))
        elif op == "add":
            self.trace.append((value, None))
        elif op == "sub":
            self.trace.append((value, None))
        elif op == "inc":
            self.trace.append((+1, None))
        elif op == "dec":
            self.trace.append((-1, None))
        elif op == "incd":
            self.trace.append((+2, None))
        elif op == "decd":
            self.trace.append((-2, None))
        elif op == "call":
            if callee != 0:    # skip unresolved weak refs
                self.trace.append((-2, callee))
                self.callees.add(callee)

    def runTrace(self):
        if self.calculatedStackUsage:
            # return memoized value
            return self.calculatedStackUsage

        selfStackUsage = 0
        worstStackUsage = selfStackUsage
        
        for (usage, callee) in self.trace:
            if callee and self.name in noRecursionFunctions:
                # do not try to do deeper
                continue
            usage = -usage   # stack "grows" to the bottom (lower addresses)
            selfStackUsage += usage
            if selfStackUsage > worstStackUsage:
                worstStackUsage = selfStackUsage
            if callee:
                if callee not in functions:
                    # warning was already printed
                    selfStackUsage -= usage
                    continue
                calleeFun = functions[callee]
                childStackUsage = selfStackUsage + calleeFun.runTrace()
                if childStackUsage > worstStackUsage:
                    worstStackUsage = childStackUsage
                # stack is automatically freed after the call returns
                selfStackUsage -= usage

        if verbose:
            prettyName = self.name.ljust(24)
            print("Function " + prettyName + " at " + hex(self.address) + ":\tworst-case stack usage is " \
                      + str(worstStackUsage) + " bytes")
        self.calculatedStackUsage = worstStackUsage
        return worstStackUsage

#########################################

if len(sys.argv) < 3:
    print('Usage: ' + sys.argv[0] + ' <target> <arch> [<verbose>]')
    sys.exit(1)

target = sys.argv[1]
arch = sys.argv[2]
verbose = len(sys.argv) > 3

# find compiler and objdump executables
if arch == 'pc':
    objdump = 'objdump'
elif arch == 'msp430':
    objdump = 'msp430-objdump'
elif arch == 'avr':
    objdump = 'avr-objdump'
else:
    print("Error: unknown arhitecture!")
    sys.exit(1)

interruptFunctions = ["alarmTimerInterrupt0", "alarmTimerInterrupt1", "sleepTimerInterrupt",
                      "userButtonInterrupt", "USCI0InterruptHandler", "USCI1InterruptHandler",
                      "UART0InterruptHandler", "UART1InterruptHandler", "USCIAInterruptHandler",
                      "mrf24Interrupt", "cc1011Interrupt", "cc2420Interrupt", "apds_interrupt",
                      "i2c_tx_interrupt"]

# TODO: make sure that interrupt handlers never use callbacks when in threaded execution!
functionsWithAllowedCallbacks = interruptFunctions + [
    "alarmsProcess", # executed in kernel-thread context only where stack is (relatively) unlimited
    # libc functions:
    "print_field", "vuprintf",
    # thread functions
    "threadWrapper"
    ]

threadInternalFunctions = [
    "startThreads", "threadCreate", "schedule",
    "serialSendByte" # because of STACK_GUARD
]

noRecursionFunctions = [
    "schedule",
    "serialSendByte" # because of STACK_GUARD
]

functionStart = Regexp(r'([0-9a-fA-F]+)\s<([a-zA-Z_][a-zA-Z_0-9\.]*)>:')
callInstr     = Regexp(r'.*call\s+#(0x[0-9a-fA-F]+)')
ptrCallInstr  = Regexp(r'.*call\s+([^\s]+)\s')
pushPopInstr  = Regexp(r'.*((push)|(pop))\s+(r[0-9][0-9]?)')
arithmInstr   = Regexp(r'.*((add)|(sub))\s+#(-?[0-9]+),\s+r1\s')
shortArithmInstr   = Regexp(r'.*((inc)|(dec)|(incd)|(decd))\s+r1\s')
explicitStackManipulation  = Regexp(r'.*\s+r1\s|,')


functions = {}
functionsByName = {}

haveThreads = False
stackPerThread = 512

def analyzeObjFile():
    global haveThreads
    global stackPerThread

    symbols = os.popen(objdump + " -t " + target).readlines()
    bssSymbol = Regexp(r'([0-9a-f]{8}).+\.bss\s+([0-9a-f]{8})\s+([^\s]+)')
    threadSize = 0
    threadStackSize = 0
    for line in symbols:
        if bssSymbol.match(line):
            address = bssSymbol.group(1)
            size = int(bssSymbol.group(2), 16)
            name = bssSymbol.group(3)
            # print name  + " at " + address + " is " + hex(size)            
            if name == "threadStackBuffer":
                threadStackSize = size
            elif name == "threads":
                threadSize = size

    if threadSize and threadStackSize:
        haveThreads = True
        # 12 is the lower bound on single threads size
        numThreads = threadSize / 12 - 1 # do not include kernel's thread
        stackPerThread = threadStackSize / numThreads

def buildTrace():
    ignoreFunctions = ["__ctors_end"]
    ignoreMode = False
    currentFunction = None

    binaryCode = os.popen(objdump + " -d " + target).readlines()
    for line in binaryCode:
        if functionStart.match(line):
            # save the old current function
            if currentFunction:
                functions[currentFunction.address] = currentFunction
                functionsByName[currentFunction.name] = currentFunction

            funAddress = int(functionStart.group(1), 16)
            funName = functionStart.group(2).strip()
            #print "\nfunction:", funName, "at", hex(funAddress)
            if funName in ignoreFunctions:
                ignoreMode = True
            else:
                # make new function and set as the current
                currentFunction = Function(funName, funAddress)
                ignoreMode = False
                continue

        if ignoreMode:
            continue

        if callInstr.match(line):
            #print "call: ", callInstr.group(1)
            callee = int(callInstr.group(1), 16)
            currentFunction.addOp("call", 0, callee)
        elif ptrCallInstr.match(line):
            if currentFunction.name in functionsWithAllowedCallbacks:
                continue
            print("Warning! Unhandled call-by-pointer in function " + currentFunction.name)
            print("Stack usage analysis results may be incorrect.")
        elif pushPopInstr.match(line):
            #print "push/pop:", line.strip()
            #print pushPopInstr.group(1)
            currentFunction.addOp(pushPopInstr.group(1))
        elif arithmInstr.match(line):
            #print "add/sub: ", line.strip()
            #print arithmInstr.group(1)
            #print arithmInstr.group(4)
            currentFunction.addOp(arithmInstr.group(1), int(arithmInstr.group(4)))
        elif shortArithmInstr.match(line):
            #print "inc/dec: ", line.strip()
            #print shortArithmInstr.group(1)
            currentFunction.addOp(shortArithmInstr.group(1))
        elif explicitStackManipulation.match(line):
            if currentFunction.name == "__init_stack" \
                    or currentFunction.name in threadInternalFunctions:
                continue
            print("Warning! Unhandled stack-related instruction in function " \
                      + currentFunction.name)
            print("Stack usage analysis results may be incorrect. The instruction was:")
            print(line)

    # save the old current function
    if currentFunction:
        functions[currentFunction.address] = currentFunction
        functionsByName[currentFunction.name] = currentFunction

def detectLoops(backtrace):
    last = backtrace[-1]
    if last not in functions:
        print "unknown function {:#x}".format(last)
        return
    nonrecursiveCallees = []
    for callee in functions[last].callees:
        if callee in backtrace:
            if functions[last].name in noRecursionFunctions:
                # print "skip " + functions[last].name
                continue
            print("Recursion detected, aborting stack checks! Stack backtrace:")
            for f in backtrace:
                print("    " + hex(f) + ":    " + functions[f].name + "()")
            print("Recursive call to function " + functions[callee].name + "()!")
            sys.exit(0)
        nonrecursiveCallees.append(callee)
        detectLoops(backtrace + [callee])
    functions[last].callees = nonrecursiveCallees

def simulateWorstCaseStackUsage():
    if haveThreads:
        wsu = functionsByName["appMain"].runTrace()
        # TODO: other user threads (find by: call and args to threadCreate())
        # TODO: also include kernel thread?
        wsu += 2 # compensate for a single call overhead (in threadWrapper)
    else:
        wsu = functionsByName["main"].runTrace()
    
    worstInterruptUsage = 0
    hasInts = False
    for interruptFunction in interruptFunctions:
        if interruptFunction in functionsByName:
            hasInts = True
            usage = functionsByName[interruptFunction].runTrace()
            if usage > worstInterruptUsage:
                worstInterruptUsage = usage
    if hasInts:
        # interrupt call overhead is 4, because not only return address, but also
        # status register is pushed onto stack!
        wsu += 4
        wsu += worstInterruptUsage

    maybeUserThreads = " in user threads" if haveThreads else ""
    print("Estimated worst-case stack usage" + maybeUserThreads \
              + ": " + str(wsu) + " bytes (incl. " \
              + str(worstInterruptUsage) + " bytes in interrupts)")

    if stackPerThread < wsu:
        print("Error: thread stack space is insufficient: " + str(stackPerThread) + " bytes allocated, " \
            + str(wsu) + " bytes required\n")
        sys.exit(1)
    print("")
    

def main():
    if verbose: print("")
    analyzeObjFile()
    buildTrace()
    if "main" not in functionsByName:
        print("Aborting: main() function not defined?!")
        return
    detectLoops([functionsByName["main"].address])
    simulateWorstCaseStackUsage()

if __name__ == '__main__':
    main()
    
