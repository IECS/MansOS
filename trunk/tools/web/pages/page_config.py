#!/usr/bin/env python

import os

import sensor_data
import helper_tools as ht
import moteconfig
from motes import motes
import configuration
import json
import utils

class PageConfig():
    def serveConfig(self, qs):
        global isListening

        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        
        if "platform_set" in qs:
            for m in motes.getMotes():
                motename = "sel_mote" + utils.urlEscape(m.getFullBasename())
                if motename in qs:
                    m.platform = qs[motename][0]
            motes.storeSelected()
            text = '<strong>Mote platforms updated!</strong>\n'
            self.writeChunk(self.serveAnyPage("config", qs, content = text, generatedContentOnly = True))
            return
        else:
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
                self.writeChunk("<h4 class='err'>Config page requested, but mote not specified!</h4>")
                return

            motePortNameEsc = utils.urlEscape(motePortName)
    
            platform = None
            dropdownName = "sel_mote" + motePortNameEsc
            if dropdownName in qs:
                platform = qs[dropdownName][0]
    
            if platform not in moteconfig.supportedPlatforms:
                self.writeChunk("<h4 class='err'>Config page requested, but platform not specified or unknown!</h4>")
                return

            moteconfig.instance.setMote(motes.getMote(motePortName), platform)
    
            (errmsg, ok) = moteconfig.instance.updateConfigValues(qs)
            if not ok:
                self.writeChunk(errmsg)
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
                    (text, ok) = moteconfig.instance.getFileListHTML(motePortNameEsc)
            else:
                (text, ok) = moteconfig.instance.getConfigHTML()
            if not ok:
                self.writeChunk("\n<h4 class='err'>Error: " + text + "</h4>\n")
                return
                
            moteidQS = "?sel_mote" + motePortNameEsc + "=" + platform + "&" + "mote" + motePortNameEsc
            replaceValues = {
                "MOTEID_CONFIG" : moteidQS + "_cfg=1",
                "MOTEID_FILES" : moteidQS + "_files=1"
            }
            
            if "update_sensors" in qs:
                configuration.c.selectSection("graph")
                graphConfig = []
                storeConfig = []
                try:
                    graphSensors = json.loads(configuration.c.getCfgValue("graphsensors"))
                except:
                    graphSensors = {}
                for s in moteconfig.instance.activePlatform.sensors:
                    storeFlag = "store_" + s.varname
                    graphFlag = "graph_" + s.varname
                    if storeFlag in qs and qs[storeFlag][0] == 'on':
                        storeConfig.append(s.varname)
                    if graphFlag in qs and qs[graphFlag][0] == 'on':
                        graphConfig.append(s.varname)
                configuration.c.selectSection("graph")
                graphSensors[motePortNameEsc] = graphConfig
                configuration.c.setCfgValue("graphsensors", json.dumps(graphSensors))
                configuration.c.save()
            else:
                try:
                    configuration.c.selectSection("graph")
                    graphSensors = json.loads(configuration.c.getCfgValue("graphsensors"))
                    graphConfig = graphSensors[motePortNameEsc]
                except:
                    graphConfig = []
                for s in moteconfig.instance.activePlatform.sensors:
                    if s.varname in graphConfig:
                        replaceValues["graph_" + s.varname] = "checked"
                    else:
                        replaceValues["graph_" + s.varname] = ""
                
            self.writeChunk(self.serveAnyPage("config", qs, isGeneric = False, content = text, replaceValues = replaceValues, generatedContentOnly = True))
