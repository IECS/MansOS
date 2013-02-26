import glob

def comports():
    devices = glob.glob('/dev/com*')
    return [(d, d, d) for d in devices]
