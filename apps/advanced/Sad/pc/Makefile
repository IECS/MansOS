
.PHONY: all client clean

all: client

# CC=mips-linux-uclibc-gcc

client:
	mkdir -p build
	$(CC) -g client.c ../crc.c -o build/client

clean:
	-@find . -type d -name build -exec rm -rf \{\} \; 
