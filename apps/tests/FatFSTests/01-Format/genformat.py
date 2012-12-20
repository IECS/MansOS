#!/usr/bin/python

import struct

PARTITIONED_IMAGE_NAME = "formatted-part-gen.img"
NONPARTITIONED_IMAGE_NAME = "formatted-nonpart-gen.img"

BLOCK_SIZE = 512
PARTITION_OFFSET_BLOCKS = 2048
PARTITION_OFFSET = PARTITION_OFFSET_BLOCKS * BLOCK_SIZE
TOTAL_SIZE = 16 * 1024 * 1024
TOTAL_BLOCKS = TOTAL_SIZE / BLOCK_SIZE

def writeZeros(f, num):
    f.write("\x00" * num)

def totalSectors(isPartitioned):
    ret = TOTAL_BLOCKS
    if isPartitioned: ret -= PARTITION_OFFSET_BLOCKS
    return ret

def genSuperblock(f):
    writeZeros(f, 440)
    f.write("\x2d\x0e\x60\x8c\x00\x00") # 6 bytes of garbage
    # partition #1
    # begin/end baliues are more or less random - assume the are not going to be used
    f.write("\x00") # boot flag
    f.write("\x21") # begin head
    f.write("\x03") # begin sector, begin cylinder high
    f.write("\x00") # begin cylinder low

    f.write("\x83") # partition type (Linux)
    f.write("\x21") # end head
    f.write("\x03") # end sector, end cylinder high
    f.write("\x00") # end cylinder low

    f.write(struct.pack("<I", PARTITION_OFFSET_BLOCKS))  # first sector
    f.write(struct.pack("<I", TOTAL_BLOCKS - PARTITION_OFFSET_BLOCKS)) # total sectors
    # partitions #2, #3, #4 - all zeros
    writeZeros(f, 16 * 3)
    f.write("\x55\xAA")  # end marker
    writeZeros(f, 1048064)

def genFile(filename, isPartitioned):
    f = open(filename, "w")
    if isPartitioned:
        genSuperblock(f)

    f.write("\xeb\x3c\x90\x6d\x6b\x64\x6f\x73\x66\x73\x00")
    f.write("\x00\x02") # bytes per sector (512)
    f.write("\x40")     # sectors per cluster (64)
    f.write("\x40\x00") # reserved sectors count (64, must not be zero)
    f.write("\x02")     # FAT count (2)
    f.write("\x00\x04") # root dir entry count (2048)
    f.write("\x00\x00") # total sectors-16
    f.write("\xf8")     # media type - fixed (non-removable) media (?)
    f.write("\x00\x01") # sectors per fat (512)
    f.write("\x3e\x00") # sectors per track
    f.write("\x3e\x00") # head count
    f.write("\x00\x00\x00\x00") # hidden sectors
    f.write(struct.pack("<I", totalSectors(isPartitioned)))  # total sectors-32
    f.write("\x00\x00") # physical drive number
    f.write("\x29")     # boot record signature?
    f.write("\xf9\xef\xdb\xc0")  # volume serial number
    f.write("\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20") # disk label
    f.write("\x46\x41\x54\x31\x36\x20\x20\x20") # file system-ID (FAT 16)
    # boot code starts here?
    f.write("\x0e\x1f")
    f.write("\xbe\x5b\x7c\xac\x22\xc0\x74\x0b\x56\xb4\x0e\xbb\x07\x00\xcd\x10")
    f.write("\x5e\xeb\xf0\x32\xe4\xcd\x16\xcd\x19\xeb\xfe\x54\x68\x69\x73\x20")
    f.write("\x69\x73\x20\x6e\x6f\x74\x20\x61\x20\x62\x6f\x6f\x74\x61\x62\x6c")
    f.write("\x65\x20\x64\x69\x73\x6b\x2e\x20\x20\x50\x6c\x65\x61\x73\x65\x20")
    f.write("\x69\x6e\x73\x65\x72\x74\x20\x61\x20\x62\x6f\x6f\x74\x61\x62\x6c")
    f.write("\x65\x20\x66\x6c\x6f\x70\x70\x79\x20\x61\x6e\x64\x0d\x0a\x70\x72")
    f.write("\x65\x73\x73\x20\x61\x6e\x79\x20\x6b\x65\x79\x20\x74\x6f\x20\x74")
    f.write("\x72\x79\x20\x61\x67\x61\x69\x6e\x20\x2e\x2e\x2e\x20\x0d\x0a\x00")

    writeZeros(f, 19 * 16)
    f.write("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xaa")

    writeZeros(f, 2016 * 16)
    # FAT table 1
    f.write("\xf8\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
    writeZeros(f, 8191 * 16)

    # FAT table 2 (copy of table 1)
    f.write("\xf8\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
    writeZeros(f, 8191 * 16)

    # fill the rest of image with zeros (empty filesystem)
    rest = TOTAL_SIZE
    rest -= 0x48000
    if isPartitioned:
        rest -= PARTITION_OFFSET
    writeZeros(f, rest)

    # finished with the image
    f.close()

def main():
    genFile(PARTITIONED_IMAGE_NAME, True)
    genFile(NONPARTITIONED_IMAGE_NAME, False)
    print "images generated!"


if __name__ == '__main__':
    main()
