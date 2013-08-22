
from __future__ import print_function
import random

class PageUser():
    def serveEditUsers(self, qs):
        tses = self.sessions.get_session(qs["sma"][0])
        webAttributes = self.settings.getCfgValue("adminWebAttributes")
        if isinstance(webAttributes, str):
            webAttributes = [webAttributes]
        user = self.users.get_user("name", tses.from_code(qs["edituser"][0]))
        if not user:
            del qs["edituser"]
            self.serveUsers(qs, True)
            return
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        changes = {}
        if self.isSafe():
            formCode = "<form>"
            formCode += "<p> Name: <strong>" + tses.to_code(user.get("name", "")) + "</strong>"
            tcod = str(random.randint(10000000, 99999999))
            formCode += "<input type='hidden' class='coded tocode' id='" + tcod
            formCode += "' value=\"" + tses.to_code(user.get("name", ""), False, tcod) + "\" name='name'></p>"
            for atr in webAttributes:
                tcod = str(random.randint(10000000, 99999999))
                if atr in ["name", "password"]:
                    continue
                else:
                    ttype = self.getAttributeType(atr)
                    if type(ttype) is list:
                        formCode += "<p><input autocomplete='off' type='hidden' class='coded tocode list' id='" + tcod
                        formCode += "' value=\"" + tses.to_code(user.get(atr, ""), False, tcod) + "\" name='" + atr + "'>"
                        formCode += atr +": <select id='" + atr + "'>"
                        for opt in ttype:
                            formCode += "<option value=" + opt + ">" + opt + "</option>"
                        formCode += "</select></p>"
                    elif ttype == "bool":
                        formCode += "<p><input autocomplete='off' type='hidden' class='coded tocode bool' id='" + tcod
                        formCode += "' value=\"" + tses.to_code(user.get(atr, ""), False, tcod) + "\" name='" + atr + "'>"
                        formCode += "<input type='checkbox' class='checkbox' id='" + atr + "'>" + atr + "</p>"
                    else:
                        formCode += "<p>" + atr + ": " + "<input autocomplete='off' type='text' class='coded tocode' id='" + tcod
                        formCode += "' value=\"" + tses.to_code(user.get(atr, ""), False, tcod) + "\" name='" + atr + "'></p>"
            formCode += "<p><input type='checkbox' class='md5' id='yesplease' name='password'>Reset passsord.</p>"
            formCode += "<p><input type='hidden' id='randtextsave' name='saveuser'>"
            formCode += "<input type='submit' onclick='return userSave()' value='Save'>"
            formCode += "<input type='hidden' id='deleteuser' name='delete'>"
            formCode += "<input type='submit' onclick='return userDelete()' value='Delete user'></p>"
            formCode += "</form>"
            changes["FORM"] = formCode
        else:
            changes["FORM"] = ''

        content = self.serveBody("edituser", qs, changes)
        self.serveAnyPage("edituser", qs, title = "Edit user", content = content)   

    def serveAddUsers(self, qs, info = ''):
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        changes = {}
        changes["INFO"] = info
  
        content = self.serveBody("adduser", qs, changes)
        self.serveAnyPage("adduser", qs, title = "Add user", content = content)      

    def serveUsers(self, qs, isSession = False):
        if not isSession:
            self.setSession(qs)
        if not self.getLevel() > 7:
            self.serveDefault(qs, True)
            return
        tses = self.sessions.get_session(qs["sma"][0])
        webAttributes = self.settings.getCfgValue("adminWebAttributes")
        changes = {}
        changes["INFO"] = ''
        changes["PSW"] = ''
        changes["ADD"] = ''
        if self.getLevel() > 8:
            changes["ADD"] = '<a href="/users?useradd=true"><button>Add new user</button></a>'
            if "delete" in qs:
                #delete users
                if qs["delete"][0] == tses.to_md5("deleteuser") and self.isSafe() and "name" in qs:
                    user = self.users.get_user("name", tses.from_code(qs["name"][0]))
                    if user:
                        if not user.get("name", "") in self.tabuList:
                            if self.users.del_user(tses.from_code(qs["name"][0])):
                                changes["INFO"] = "<h4 class='suc'> Deleted user " + tses.to_code(tses.from_code(qs["name"][0])) + "! </h4>"
                                self.users.write_in_file()
                            else:
                                changes["INFO"] = "<h4 class='err'> Could not delete user " + tses.to_code(tses.from_code(qs["name"][0])) + "! </h4>"
                        else:
                            changes["INFO"] = "<h4 class='err'> It is not allowed to delete " + tses.to_code(tses.from_code(qs["name"][0])) + "! </h4>"
                    else:
                        changes["INFO"] = "<h4 class='err'> Could not delete user " + tses.to_code(tses.from_code(qs["name"][0])) + "! </h4>"
                else:
                    changes["INFO"] = "<h4 class='err'> Wrong data for deleting! </h4>"
            elif "saveuser" in qs:
                #save changes
                if qs["saveuser"][0] == tses.to_md5("randtextsave") and self.isSafe() and "name" in qs:
                    user = self.users.get_user("name", tses.from_code(qs["name"][0]))
                    if user:
                        username = user["name"]
                        for atr in webAttributes:
                            if not atr in qs:
                                continue
                            if atr in ["name", "password"]:
                                continue
                            if atr == "level" and username in self.tabuList:
                                continue
                            self.users.set_attribute(username, atr, tses.from_code(qs[atr][0]))
                        changes["INFO"] = "<h4 class='suc'> Saved user " + tses.to_code(username) + "! </h4>"
                        #password
                        if "password" in qs:
                            if qs["password"][0] == tses.to_md5("yesplease"):
                                if username in self.tabuList:
                                    changes["PSW"] = "<h4 class='err'>  Can not reset " + tses.to_code(username) + " password.</h4>"
                                elif "level" in qs:
                                    if tses.from_code(qs["level"][0]) == "9":
                                        changes["PSW"] = "<h4 class='err'>  Can not reset level 9 password.</h4>"
                                    else:
                                        changes["PSW"] = "<h4 class='suc'>  Password is " + tses.to_code(self.users.set_psw(username)) + ".</h4>"
                                else:
                                    tuser = self.users.get_user("name", qs["user"][0])
                                    if "level" in tuser and tuser["level"] == "9":
                                        changes["PSW"] = "<h4 class='err'>  Can not reset level 9 password.</h4>"
                                    else:
                                        changes["PSW"] = "<h4 class='suc'>  Password is " + tses.to_code(self.users.set_psw(username)) + ".</h4>"
                        self.users.write_in_file()
                    else:
                        changes["INFO"] = "<h4 class='err'> Could not save user " + tses.to_code(tses.from_code(qs["name"][0])) + "! </h4>"
                else:
                    changes["INFO"] = "<h4 class='err'> Wrong data for saving! </h4>"
            elif "useradd" in qs:
                if "username" in qs:
                    tusername = tses.from_code(qs["username"][0])
                    if self.users.get_user("name", tusername):
                        self.serveAddUsers(qs,"<h4 class='err'> User with name " + tses.to_code(tusername) + " already exists! </h4>")
                    else:
                        dv = self.settings.getCfgValue("defaultValues")
                        ua = self.settings.getCfgValue("userAttributes")
                        userdata = []
                        for atr in self.users._userAttributes:
                            if atr == "name":
                                userdata.append(tusername)
                                continue
                            i = ua.__len__() - 1
                            while i > -1:
                                if atr == ua[i]:
                                    userdata.append(dv[i])
                                    break
                                i -= 1
                            if i == -1:
                                userdata.append("None")
                        if self.users.add_user(userdata):
                            changes["INFO"] = "<h4 class='suc'> New user " + tses.to_code(tusername) + " added!</h4>"
                            changes["PSW"] = "<h4 class='suc'>  Password is " + tses.to_code(self.users.set_psw(tusername)) + ".</h4>"
                            self.users.write_in_file()
                        else:
                            changes["INFO"] = "<h4 class='err'> Could not add user " + tses.to_code(tusername) + "! </h4>"
                else:
                    self.serveAddUsers(qs)
                    return
            elif "edituser" in qs:
                self.serveEditUsers(qs)
                return
        self.send_response(200)
        self.sendDefaultHeaders()
        self.end_headers()
        if self.isSafe():
            #user tabula
            tabCode = "<table class='table' id='usertable'><tr>"
            tabCode += "<th>" + tses.to_code("name") + "</th>"
            for atr in webAttributes:
                if atr in ["name"]:
                    continue
                tabCode += "<th>" + tses.to_code(atr) + "</th>"
            tabCode += "</tr>"
            for user in self.users._userList:
                tabCode += "<tr>"
                tabCode += "<td>" + tses.to_code(user.get_data("name")) + "</td>"
                for atr in webAttributes:
                    if atr in ["name"]:
                        continue
                    tabCode += "<td>" + tses.to_code(user.get_data(atr)) + "</td>"
                tabCode += "</tr>"
            tabCode += "</table>"
            changes["TAB"] = tabCode
        else:
            changes["TAB"] = ''

        content = self.serveBody("users", qs, changes)
        self.serveAnyPage("users", qs, content = content)
