#!/usr/bin/env python

import os

import sensor_data
import helper_tools as ht
import moteconfig
from motes import motes

class PageConfig():
    def serveConfig(self, qs):
        self.setSession(qs)
        if not self.getLevel() > 1:
            self.serveDefault(qs, True)
            return
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        
        if "platform_set" in qs:
            for m in motes.getMotes():
                motename = "sel_mote" + m.getPortBasename()
                if motename in qs:
                    m.platform = qs[motename][0]
            motes.storeSelected()
            text = '<strong>Mote platforms updated!</strong>\n'
            self.serveAnyPage("config", qs, content = text)
            return
        else:
            global isListening
            sensor_data.moteData.reset()
            ht.openAllSerial()
            isListening = True
            
            motePortName = None
            filesRequired = False
            for s in qs:
                if s[:4] == "mote":
                    pair = s[4:].split('_')
                    try:
                        motePortName = pair[0]
                        filesRequired = pair[1] == "files"
                    except:
                        pass
                    break
    
            if motePortName is None: # or moteIndex >= len(motes.getMotes()):
                self.serveAnyPage("error:critical", qs, 
                    errorMsg = "Config page requested, but mote not specified!")
                return
    
            platform = None
            dropdownName = "sel_mote" + motePortName
            if dropdownName in qs:
                platform = qs[dropdownName][0]
    
            if platform not in moteconfig.supportedPlatforms:
                self.serveAnyPage("error:critical", qs, 
                    errorMsg = "Config page requested, but platform not specified or unknown!")
                return

            if os.name == "posix":
                fullMotePortName = "/dev/" + motePortName
            else:
                fullMotePortName = motePortName
    
            moteconfig.instance.setMote(motes.getMote(fullMotePortName), platform)
    
            (errmsg, ok) = moteconfig.instance.updateConfigValues(qs)
            if not ok:
                self.serveAnyPage("error:critical", qs, errorMsg = errmsg)
                return

            # fill config values from the mote / send new values to the mote
            if "get" in qs:
                reply = moteconfig.instance.getConfigValues()
            elif "set" in qs:
                reply = moteconfig.instance.setConfigValues()

            if filesRequired:
                if "filename" in qs:
                    (text, ok) = moteconfig.instance.getFileContentsHTML(qs)
                else:
                    (text, ok) = moteconfig.instance.getFileListHTML(motePortName)
            else:
                (text, ok) = moteconfig.instance.getConfigHTML()
            if not ok:
                self.serveAnyPage("error:critical", qs, errorMsg = text)
                return
            
            moteidQS = "?sel_mote" + motePortName + "=" + platform + "&" + "mote" + motePortName
            self.serveAnyPage("config", qs, isGeneric = False, content = text, replaceValues = {
                              "MOTEID_CONFIG" : moteidQS + "_cfg=1",
                              "MOTEID_FILES" : moteidQS + "_files=1"})