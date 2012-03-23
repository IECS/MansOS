#!/bin/sh

DIR=build
MANSOS_PATH=../../..
ARCH=telosb

cd $DIR
for f in *.c
do
    cat > ./Makefile <<EOF
SOURCES = $f
APPMOD = App
PROJDIR = \$(CURDIR)
ifndef MOSROOT
  MOSROOT = \$(PROJDIR)/$MANSOS_PATH
endif
include \${MOSROOT}/mos/make/Makefile
EOF
    make clean && make $ARCH
done