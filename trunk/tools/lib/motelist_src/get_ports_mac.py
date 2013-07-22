import glob

def comports():
    """scan for available ports. return a list of device names."""
    devices = glob.glob('/dev/tty.*')
    return [(d, d, d) for d in devices]
