from testarch import *

class UncommonSensor(SealSensor):
    def __init__(self):
        super(UncommonSensor, self).__init__("UncommonSensor")
        self.useFunction.value = "1337"
        self.readFunction.value = self.useFunction.value

uc = UncommonSensor()
