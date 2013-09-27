
from __future__ import print_function
import random

class PageGraph():
    def serveGraphs(self, qs):
# TODO
#        isMobile = "Android" in self.headers["user-agent"]
        isMobile = False
        self.setSession(qs)
        changes = {}
        changes["INFO"] = ''
        changes["ADD"] = ''
        if self.getLevel() > 1 and self.isSafe():
            if isMobile == True:
                changes["ADD"] = "<a href='/graph?addnewgraph=true'><button type='button' style='width: 100%' class='btn btn-primary' data-toggle='button'>Add new graph</button></a>"
            else:
                changes["ADD"] = '<a href="/graph?addnewgraph=true"><button>Add new graph</button></a>'
            
            if "editgraph" in qs:
                self.serveEditGraph(qs)
                return
            tses = self.sessions.get_session(qs["sma"][0])
            graphAttributes = self.settings.getCfgValueAsList("graphAttributes")
            if "savegraph" in qs:
                #save changes
                if qs["savegraph"][0] == tses.to_md5("GraphSettingsSave") and self.isSafe():
                    number = int(tses.from_code(qs["graph"][0]))
                    for atr in graphAttributes:
                        if not atr in qs:
                            continue
                        tsettings = self.settings.getCfgValueAsList(atr)
                        if isinstance(tsettings[0], list):
                            tsettings[number] = tses.from_code(qs[atr][0]).translate(None, " ").split(",")
                        else:
                            tsettings[number] = tses.from_code(qs[atr][0])
                        self.settings.setCfgValue(atr, tsettings)
                    changes["INFO"] = "<h4 class='suc'> Saved changes! </h4>"
                    self.settings.save()
                else:
                    changes["INFO"] = "<h4 class='err'> Wrong data for saving! </h4>"
            elif "delete" in qs:
                #delete graph
                if qs["delete"][0] == tses.to_md5("deletegraph") and self.isSafe() and "graph" in qs:
                    number = int(tses.from_code(qs["graph"][0]))
                    if len(self.settings.getCfgValue(graphAttributes[0])) > 1:
                        if len(self.settings.getCfgValue(graphAttributes[0])) > number:
                            for atr in graphAttributes:
                                tsettings = self.settings.getCfgValueAsList(atr)
                                tsettings.pop(number)
                                self.settings.setCfgValue(atr, tsettings)
                            changes["INFO"] = "<h4 class='suc'> Deleted graph! </h4>"
                            self.settings.save()
                        else:
                            changes["INFO"] = "<h4 class='err'> There isn't graph " + tses.to_code(str(number)) + "! </h4>"
                    else:
                        changes["INFO"] = "<h4 class='err'> Can't delete last graph! </h4>"
                else:
                    changes["INFO"] = "<h4 class='err'> Wrong data for deleting! </h4>"
            elif "addnewgraph" in qs:
                if qs["addnewgraph"][0] != "true":
                    if  qs["addnewgraph"][0] == tses.to_md5("addnewgraph") and self.isSafe():
                        for atr in graphAttributes:
                            tsettings = self.settings.getCfgValueAsList(atr)
                            tsettings.append(tses.from_code(qs[atr][0]));                          
                            self.settings.setCfgValue(atr, tsettings)
                        changes["INFO"] = "<h4 class='suc'> New graph added!</h4>"
                        self.settings.save()
                    else:
                        changes["INFO"] = "<h4 class='err'> Wrong data for adding new graph! </h4>"
                else:
                    self.serveAddGraph(qs)
                    return
                
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        if isMobile == True:
            if self.getLevel() > 1:
                buttons = "<table width='100%'>"
                buttons += "<tr><td><button type='button' style='width: 100%' class='btn btn-primary' data-toggle='button' onclick='button(\" + i + \", this)'>Pause</button></td></tr>"
                buttons += "<tr><td><a href='/graph?editgraph=\" + i + \"'><button type='button' style='width: 100%' class='btn btn-primary' data-toggle='button'>Edit graph</button></a></td></tr>"
                buttons += "<tr><td>" + changes["ADD"] + "</td></tr>"
                buttons + "</table>"
            else:
                buttons = "<button type='button' class='btn btn-primary' data-toggle='button' onclick='button(\" + i + \", this)'>Pause</button>"
        else:
            if self.getLevel() > 1:
                buttons = "<button onclick='button(\" + i + \", this)'>Pause</button><a href='/graph?editgraph=\" + i + \"'><button>Edit graph</button></a>"
            else:
                buttons = "<button onclick='button(\" + i + \", this)'>Pause</button>"

        changes["BUTTONS"] = buttons
        content = self.serveBody("graph", qs, changes)
        self.serveAnyPage("graph", qs, content = content, replaceValues = changes)

    def serveAddGraph(self, qs):
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        changes = {}
        changes["FORM"] = ''
        if self.getLevel() > 1:
            graphAttributes = self.settings.getCfgValueAsList("graphAttributes")
            formCode = "<form>"
            for atr in graphAttributes:
                formCode += "<p>" + atr + ": " + "<input autocomplete='off' type='text' class='tocode must widthtext' "
                formCode += "name='" + atr + "'></p>"
            formCode += "<input type='hidden' id='addnewgraph' name='addnewgraph'>"
            formCode += "<input type='submit' onclick='return addGraph()' value='Add graph'></p>"
            formCode += "</form>"
            changes["FORM"] = formCode
            
        content = self.serveBody("addgraph", qs, changes)
        self.serveAnyPage("addgraph", qs, title = "Add graph", content = content)

    def serveEditGraph(self, qs):
        tses = self.sessions.get_session(qs["sma"][0])
        graphAttributes = self.settings.getCfgValueAsList("graphAttributes")
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        changes = {}
        if self.isSafe():
            formCode = "<form>"
            number = int(qs["editgraph"][0])
            formCode += "<p> Graph: <strong>" + tses.to_code(self.settings.getCfgValueAsList("graphTitle")[number]) + "</strong>"
            tcod = str(random.randint(10000000, 99999999))
            formCode += "<input type='hidden' class='coded tocode' id='" + tcod
            formCode += "' value=\"" + tses.to_code(qs["editgraph"][0], False, tcod) + "\" name='graph'></p>"
            for atr in graphAttributes:
                tcod = str(random.randint(10000000, 99999999))
                val = self.settings.getCfgValueAsList(atr)[number]
                if isinstance(val, list):
                    tvalue = ",".join(val)
                else:
                    tvalue = val
                formCode += "<p>" + atr + ": " + "<input autocomplete='off' type='text' class='coded tocode widthtext' id='" + tcod
                formCode += "' value=\"" + tses.to_code(tvalue, False, tcod) + "\" name='" + atr + "'></p>"
            formCode += "<p><input type='hidden' id='GraphSettingsSave' name='savegraph'>"
            formCode += "<input type='submit' onclick='return graphSave()' value='Save'>"
            formCode += "<input type='hidden' id='deletegraph' name='delete'>"
            formCode += "<input type='submit' onclick='return graphDelete()' value='Delete graph'></p>"
            formCode += "</form>"
            changes["FORM"] = formCode
        else:
            changes["FORM"] = ''

        content = self.serveBody("editgraph", qs, changes)
        self.serveAnyPage("editgraph", qs, title = "Edit graph", content = content)


    def serveGraphData(self, qs):
        global lastData

        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()

        # get the data to display in graphs
        allData = ""
        if self.moteData.hasData():
            data = self.moteData.getData()
            for mote in data:
                for sensor in mote.keys():
                    allData += sensor + ":"
                    for measur in mote[sensor]:
                        allData += str(measur[0]) + "," + str(measur[1]) + ";"
                    allData += "|"
        lastData = allData
        self.writeChunk(allData)
        
    def serveGraphForm(self, qs):
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        tgraphForm = []
        settings = self.settings.getCfgValueAsList("graphTitle")
        graphlen = len(settings)
        for setting in settings:
            tgraphForm.append(setting + ":")
        i = 0
        settings = self.settings.getCfgValueAsList("graphYAxis")
        while i < graphlen:
            tgraphForm[i] += settings[i] + ";"
            i += 1
        i = 0
        settings = self.settings.getCfgValueAsList("graphInterval")
        while i < graphlen:
            tgraphForm[i] += settings[i] + ";"
            i += 1
        i = 0
        settings = self.settings.getCfgValueAsList("graphData")
        while i < graphlen:
            tdata = ""
            if isinstance(settings[i], list):
                for data in settings[i]:
                    tdata += data + ";"
            else:
                tdata += settings[i] + ";"
            tgraphForm[i] += tdata
            i += 1
            
        graphForm = ""
        i = 0
        while i < graphlen:
            graphForm += tgraphForm[i] + "|"
            i += 1
        
        self.writeChunk(graphForm)
