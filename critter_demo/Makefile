# Simple Makefile for a Nintendo DS (ARM9) homebrew project
# Requires devkitPro (devkitARM + libnds)

TARGET := critter_demo
BUILD  := build
SOURCES := source
INCLUDES :=

# Optional: place data or graphics assets here
DATA :=
GRAPHICS :=

# Common compile flags
CFLAGS := -O2 -g -Wall -Wextra -fomit-frame-pointer
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions

# Link with default libraries provided by libnds
LIBS := -lnds9

# Pull in the NDS build rules from devkitARM
include $(DEVKITARM)/ds_rules 
