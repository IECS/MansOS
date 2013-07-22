import threading, datetime, os, random, md5

class User():
    def __init__(self, userAttributes, data):
        self._attributes = {}
        i=0
        while data.__len__() > i:
            self._attributes[userAttributes[i]] = data[i]
            i+=1
    def set_attributes(self, key, value):
        self._attributes[key] = value
    def get_data(self, key):
        return self._attributes.get(key, False)
    def get_all_data(self):
        return self._attributes
    
class Users():
    def __init__(self, userAttr, userDirectory, userFile):
        self._userDirectory = userDirectory
        self._userFile = userFile
        self._userAttributes = userAttr
        self._userList = []
        threading.Timer(86400, self.make_copy_24h).start() #1d = 86400s
        self._isChange = False
    def make_copy_24h(self):
        if self._isChange:
            print("User file copy made in " + self.make_copy())
        self._isChange = False
        threading.Timer(86400, self.make_copy_24h).start() #1d = 86400s
    def is_attribute(self, attrName):
        i=0
        while self._userAttributes.__len__() > i:
            if self._userAttributes[i] == attrName:
                return True
            i +=1
        return False
    def get_user(self, key, value):
        if not self.is_attribute(key):
            return False
        i=0
        while self._userList.__len__() > i:
            if self._userList[i].get_data(key) == value:
                return self._userList[i].get_all_data()
            i +=1
        return False
    def del_user(self, name):
        i=0
        while self._userList.__len__() > i:
            if self._userList[i].get_data("name") == name:
                del self._userList[i]
                return  True
            i +=1
        return False
        
    def add_user(self, userData):
        i=self._userAttributes.__len__()-1
        while i > -1:
            if self._userAttributes[i] == "name":
                break
            i-=1
        if not self.get_user("name",userData[i]):
            self._userList.append(User(self._userAttributes, userData))
            return True
        print("Did not add user {}".format(userData[i]))
        return False
    def get_users(self):
        temp = {}
        i=0
        while self._userList.__len__() > i:
            temp[i] = self._userList[i].get_all_data()
            i+=1
        return temp
    def add_attribute(self, attrName, defaultVal):
        if self.is_attribute(attrName):
            return False
        self._userAttributes.append(attrName)
        i=0
        while self._userList.__len__() > i:
            self._userList[i].set_attributes(attrName, defaultVal)
            i+=1
        return True
    def set_attribute(self, user, attrName, value):
        if not self.is_attribute(attrName):
            return False
        tuser = self.get_user("name", user)
        if type(tuser) == dict:
            tuser[attrName] = value
            return True
        return False
    def set_psw(self, username):
        npsw = ""
        alphabet = "qwertyuioplkjhgfdsazxcvbnmMNBVCXZLKJHGFDSAQWERTYUIOP1234567890"
        i = 0
        while i < 10:
            npsw += alphabet[random.randint(0, len(alphabet)-1)]
            i += 1
        m = md5.new()
        m.update(npsw)
        self.set_attribute(username, "password", m.hexdigest())
        return npsw
    def check_psw(self):
        i=0
        while self._userList.__len__() > i:
            p = self._userList[i].get_data("password")
            if p.__len__() != 32:
                return False
            if not all(c in "0123456789abcdef" for c in p):
                return False
            i+=1
        return True
    def make_copy(self):
        tstr = "/" + self._userFile + str(datetime.datetime.now())[:22]
        tstr = tstr.replace(' ', '_')
        tstr = tstr.replace(':', '-')
        tstr = tstr.replace('.dat', '')
        tstr += ".dat"
        if not os.path.exists(self._userDirectory + "/archives"):
            os.makedirs(self._userDirectory + "/archives")
        filefrom = open(self._userDirectory + "/" + self._userFile, "r")
        fileto = open(self._userDirectory + "/archives" + tstr, "w")
        fileto.write(filefrom.read())
        filefrom.close()
        fileto.close()
        return str(self._userDirectory + "/archives" + tstr)
    def write_in_file(self):
        self._isChange = True
        f = open(self._userDirectory + "/" + self._userFile, "w")
        tstr = ""
        i=0
        while self._userAttributes.__len__() > i:
            tstr += self._userAttributes[i] + " "
            i+=1
        tstr +="\n"
        f.write(tstr)
        i=0
        while self._userList.__len__() > i:
            j=0
            tstr = ""
            while self._userAttributes.__len__() > j:
                tstr += str(self._userList[i].get_data(self._userAttributes[j])) + " "
                j += 1
            tstr += "\n"
            f.write(tstr)
            i += 1
        f.close()
