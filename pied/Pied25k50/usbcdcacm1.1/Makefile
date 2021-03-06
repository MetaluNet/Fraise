#-------------------------------------------------------------------------------
#
# A very basic Makefile to compile USB CDC ACM v1.1 for PIC18F4550 
# with SDCC 3.2.0 C-compiler
#
# Copyright (c) 2009,2013 Kustaa Nyholm
#
# NOTE because of I maintain multiple parallel SDCC installations
#      the paths to the compiler (see SDCC below) and
#      the non free libraries (see SDCCFLAGS below) are
#      hard coded, modify or remove as necessary
#
# Features:
#  - Automatic generation of dependencies for explicitly named files in SRCS
#  - All generated files goes to a separate object directory
#  - Automatic creation of object directory
#
# Notes
#  - Requires GNU Make version 3.81 or later
#  - SDCC standard libs nor Makefile are not used as prerequisites (use 'clean' build if they change)
#
# Everyone should have this
.SUFFIXES:

# This is necessary so that pk2cmd (used by 'load' script) is found and for the
# pk2cmd to find its support files (it seems to search current dir which is wrong)
PATH := ${PATH}:~/pk2cmd/pk2cmd

# The output file name
TARGET = cdcacm.hex

# The source files that make up the project go here 
SRCS =	main.c usbcdc.c printft.c
 
# The libraries that are used go here
LIBS =  libc18f.lib libio18f2550.lib libm18f.lib libsdcc.lib 

# Where to find the compiler
#SDCC = /Users/nyholku/sdcc-3.2.0/bin/sdcc
SDCCHOME = /home/arsene/pd-externals/Fraise/bin
# Where to find the compiler
SDCC = $(SDCCHOME)/bin/sdcc

# Compiler flags go here
SDCCFLAGS = -V -Wl,-m,-s18f2550.lkr -mpic16 -p18f2550 --disable-warning 85 --std-sdcc99 --obanksel=3 --use-non-free \
			-I$(SDCCHOME)/share/include/pic16 -I$(SDCCHOME)/share/non-free/include/pic16 \
			-L$(SDCCHOME)/share/non-free/lib/pic16 -L$(SDCCHOME)/share/lib/pic16

# Where to store the target/intermediate/temporary/object files
OBJDIR = ../obj

#
#-------------------------------------------------------------------------------
#
# This ensures that the object directory exists and re-creates it if necessary
#
# This requires make 3.81 or later, delete this section and all expressions that
# refer to .f if you have an older make
#
.SECONDEXPANSION: 

# Uses a .f file as a flag file in each directory   
%/.f: 
	mkdir -p $(dir $@) 
	touch $@ 
	
# dont' let make remove the flag files automatically  
.PRECIOUS: %/.f 
#
#-------------------------------------------------------------------------------
#
# Actual rules
#
# Compile the C-files
$(OBJDIR)/%.o: %.c $$(@D)/.f  
	$(SDCC) -c $(SDCCFLAGS) $< -o $@
  
# Link the compiled files and libraries    
$(OBJDIR)/$(TARGET): $(addprefix $(OBJDIR)/, $(SRCS:.c=.o)) 
	$(SDCC) $(SDCCFLAGS) -o $(OBJDIR)/$(TARGET) $(addprefix $(OBJDIR)/, $(SRCS:.c=.o)) $(LIBS)
	./load
#
#-------------------------------------------------------------------------------
#
# Automatic generation of dependencies
#
# This magic code fragment from GNU make manual uses the SDCC compiler -M option
# to create a Makefile fragment for each C-source file describing the dependencies.
#
# Traditionally these fragments have the type '.d' but SDCC seems to delete them
# when it compiles files, so I use '.dep' here.
#
# Also SDCC '-M' option produces wrong dependency for the file being compiled
# in the sense that it does not contain the path, only the filename. Hence
# the 'sed' has command has been mangled to inject the missing path to the fragment.
#

# First include the dependencies
include $(addprefix $(OBJDIR)/, $(SRCS:.c=.dep))

# Then recreate them
$(OBJDIR)/%.dep: %.c $$(@D)/.f
	 @set -e; rm -f $@; \
	  $(SDCC) -c -M $(SDCCFLAGS) $< > $@.$$$$; \
	  sed -e '1 s,^,$(OBJDIR)/,' -e 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
     rm -f $@.$$$$

#------------------------------------------------------------------------------
#
# pretty standard default target
#
all:	$(OBJDIR)/$(TARGET)
#
#-------------------------------------------------------------------------------
# 
# pretty standard clean that attempts to delete all that this Makefile may left behind
#
clean:
	rm -f $(OBJDIR)/*.rel
	rm -f $(OBJDIR)/*.lnk
	rm -f $(OBJDIR)/*.S19
	rm -f $(OBJDIR)/*.map
	rm -f $(OBJDIR)/*.mem
	rm -f $(OBJDIR)/*.asm
	rm -f $(OBJDIR)/*.rst
	rm -f $(OBJDIR)/*.sym
	rm -f $(OBJDIR)/*.lst
	rm -f $(OBJDIR)/*.o
	rm -f $(OBJDIR)/*.dep
	rm -f $(OBJDIR)/*.hex
	rm -f $(OBJDIR)/$(TARGET)
#	
# cleanall deletes all in the object directory, do not use this if target dir == source dir
cleanall:
	rm $(OBJDIR)/*

#-------------------------------------------------------------------------------
	
   
    
    
