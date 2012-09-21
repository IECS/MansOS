from testarch import *

class TunnelDataCollector(SealOutput):
    def __init__(self):
        super(TunnelDataCollector, self).__init__("TunnelDataCollector")
        self.useFunction.value = "tunnelDataCollectorSend(&tunnelDataCollectorPacket, sizeof(tunnelDataCollectorPacket))"

tunnelDataColl = TunnelDataCollector()
