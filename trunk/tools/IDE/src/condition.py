# -*- coding: utf-8 -*-
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

from comment import Comment

class Condition():
    def __init__(self, mode):
        # when, elsewhen or else
        self.__mode = mode
        # else holds empty condition
        self.__condition = ''
        # Statement instances
        self.__statements = []
        # Comments
        self.__comment = Comment()

    def setMode(self, mode):
        self.__mode = mode

    def getMode(self):
        return self.__mode

    def setCondition(self, condition):
        self.__condition = condition

    def getCondition(self):
        return self.__condition

    def setStatements(self, statements):
        self.__statements = statements

    def addStatement(self, statement):
        self.__statements.append(statement)

    def getStatements(self):
        return self.__statements

    def getStatementCode(self, prefix):
        result = ''
        for statement in self.__statements:
            result += statement.getCode(prefix) + '\n'
        return result

    def getComment(self):
        return self.__comment

    def setComment(self, comment):
        self.__comment = comment

    def getCode(self, prefix):
        result = self.getComment().getPreComments(prefix) + '\n'
        result += prefix + self.getMode() + (' ' + self.getCondition()).rstrip()
        result += ': ' + self.getComment().getPostComment(True) + '\n'
        for statement in self.getStatements():
            result += statement.getCode(prefix + '\t') + '\n'
        return result.rstrip()
