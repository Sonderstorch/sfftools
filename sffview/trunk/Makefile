#
# File:		Makefile for samples
# Author:	Robert Roebling
# Created:	1999
# Updated:	
# Copyright:	(c) 1998 Robert Roebling
#
# This makefile requires a Unix version of wxWindows
# to be installed on your system. This is most often
# done typing "make install" when using the complete
# sources of wxWindows or by installing the two
# RPM packages wxGTK.XXX.rpm and wxGTK-devel.XXX.rpm
# under Linux.
#

CC = g++ 

PROGRAM = sffview
OBJECTS = $(PROGRAM).o common.o codes.o decoder.o sfffile.o sffapp.o sffdoc.o

WXCONFIG_CPP = `wx-config --cflags`

WXCONFIG_LD = `wx-config --libs`

.SUFFIXES:	.o .cpp

.cpp.o :
	$(CC) -g -Os -c $(WXCONFIG_CPP) -o $@ $<

all:    $(PROGRAM)

$(PROGRAM):	$(OBJECTS)
	$(CC) -o $(PROGRAM) $(OBJECTS) -Wl,--as-needed $(WXCONFIG_LD)

clean: 
	rm -f *.o $(PROGRAM)
