#!/usr/bin/python

import sys, argparse, struct

PACKET_FIELDS = [
@APPLICATION_FIELDS@]

#
# Packet always starts with "magic" (2 bytes), "crc" (2 bytes), "typeMask" (4 bytes),
# after which the rest of packet fields follow (4 bytes each)
#
SMALL_HEADER_SIZE = 2 + 2
HEADER_SIZE = SMALL_HEADER_SIZE + 4

def getPacketSize():
    return HEADER_SIZE + 4 * len(PACKET_FIELDS)

def parseCommandLine():
    parser = argparse.ArgumentParser(description="RAW to CSV format converter")
    parser.add_argument('-v', '--verbose', action='store_true')
    parser.add_argument('filename')
    return parser.parse_args()

def printPacketVerbose(packet):
    for i in range(len(PACKET_FIELDS)):
        sys.stdout.write("{}={:#x}\n".format(PACKET_FIELDS[i], int(packet[i])))
    sys.stdout.write("================\n")

def printPacket(packet):
    for i in range(len(PACKET_FIELDS)):
        sys.stdout.write("{},".format(int(packet[i])))
    sys.stdout.write("\n")

def crc16Add(acc, byte):
    acc ^= byte
    acc  = (acc >> 8) | ((acc << 8) & 0xffff)
    acc ^= ((acc & 0xff00) << 4) & 0xffff
    acc ^= (acc >> 8) >> 4
    acc ^= (acc & 0xff00) >> 5
    return acc

def verifyCrc(data):
    crc = int(struct.unpack_from('H', data, 2)[0])
    allZero = True
    calcCrc = 0
    for i in range(SMALL_HEADER_SIZE, len(data)):
        b = ord(data[i])
        if b != 0: allZero = False
        calcCrc = crc16Add(calcCrc, b)
    if allZero:
        return False # empty packet
    if crc != calcCrc:
        sys.stdout.write("invalid checksum!\n")
        return False # invalid packet
    return True

def main():
    args = parseCommandLine()

    if not args.verbose:
        for i in range(len(PACKET_FIELDS)):
            sys.stdout.write(PACKET_FIELDS[i] + ",")
        sys.stdout.write("\n")

    with open(args.filename, 'rb') as inputFile:
        while True:
            try:
                data = inputFile.read(getPacketSize())
            except:
                sys.stderr.write("end of file!\n")
                break

            if not verifyCrc(data):
                break

            packet = struct.unpack_from('i' * len(PACKET_FIELDS), data, HEADER_SIZE)
            if args.verbose:
                printPacketVerbose(packet)
            else:
                printPacket(packet)

if __name__ == '__main__':
    main()
