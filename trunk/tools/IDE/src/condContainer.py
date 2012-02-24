# -*- coding: UTF-8 -*-
#
# Copyright (c) 2008-2012 the MansOS team. All rights reserved.
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
#

import globals as g

class condContainer:
    def __init__(self):
        # When and else hold condition instance who represents condition 
        self.__when = None
        self.__else = None
        # Holds array of condition instances representing elsewhen
        self.__elseWhen = []
        # Describe end statement
        self.__endComment = ''
        self.__endInlineComment = ''
        self.__oneLiner = False
        self.__identifier = g.CONDITION
    
    def setWhen(self, when):
        self.__when = when
        
    def getWhen(self):
        return self.__when
    
    def setElse(self, else_):
        self.__when = else_
        
    def getElse(self):
        return self.__else
    
    def addElseWhen(self, elseWhen):
        self.__elseWhen.append(elseWhen)
        
    def getElseWhen(self):
        return self.__elseWhen
    
    def addEndComment(self, comment):
        self.__endComment = (self.__endComment + '\n' + comment).strip()
    
    def getEndComment(self):
        return self.__endComment
    
    def setEndInlineComment(self, inlineComment):
        self.__endInlineComment = inlineComment.strip()
    
    def getEndInlineComment(self):
        return self.__endInlineComment
    
    def setOneLiner(self, oneLiner):
        self.__oneLiner = oneLiner
        
    def getOneLiner(self):
        return self.__oneLiner
    
    def getIdentifier(self):
        return self.__identifier
    
    def getCode(self, prefix):
        if self.getOneLiner() == True:
            return self.getWhen().getCode(prefix)
        result = self.getWhen().getCode(prefix) + '\n'
        for cond in self.getElseWhen():
            result += cond.getCode(prefix) + '\n'
        result += self.getElse().getCode(prefix) + '\n'
        result += (self.getEndComment() + '\n').replace('\n', '\n' + prefix)
        result += prefix +'end ' + self.getEndInlineComment()
        return result.strip()
