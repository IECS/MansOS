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

import socket
import time

def listen(host, port, pipe, debug = False):
    try:
        c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        c.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        c.bind((host, port))
        c.listen(1)
        print ("Listening to {}:{}".format(host, port))
        while 1:
            csock = c.accept()[0]
            start = time.time()
            line = csock.recv(1024).strip()
            new = line.decode().replace("&&", "##")

            if new.strip() == '':
                continue

            getStr = new[new.find(" /?") + 3:new.find("HTTP")]
            assert len(getStr), "not found in {}".format(new)

            # Parse GET
            get = dict()
            for x in getStr.split("&"):
                # TODO: fix possible errors on ?cmd&key or ?cmd=1=2&key=0
                k, v = x.split("=")
                get[k.strip()] = v.strip().replace("%20", " ").replace("{", "{\n").replace("}", "}\n").replace(";", ";\n").replace("##", "&&")
            if "sync" in get:
                pipe.send("Sync recieved")
            else:
                pipe.send(get['code'])
            if debug:
                print ("Incoming conection, parameters: {}".format(get))

            # Manage JSON callback
            cb = None
            if 'jsoncallback' in get:
                cb = get['jsoncallback']

            # Do magic
            response = 'Hello!'
            # Prepare data for response
            data = "{}{}('{}')".format('HTTP/1.0 200 OK\n\n', cb, response)

            # Respond
            csock.sendall(data.encode())

            csock.close()
            if debug:
                print ("Processed in {}".format(time.time() - start))

    except:
        print ("Socket listening thread exception occurred, terminating...")
