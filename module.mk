# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := .

LSRC := $(wildcard $(LPATH)/*.cpp)
LPRODUCTS := pentagram

pentagram_OBJ = \
	pentagram.o \
	kernel/Kernel.o \
	kernel/Application.o \
	usecode/UCMachine.o \
	usecode/UCProcess.o \
	usecode/UsecodeFlex.o \
	usecode/UCList.o \
	filesys/FileSystem.o \
	filesys/Flex.o \
	misc/Args.o \
	misc/Console.o \
	misc/Q_strcasecmp.o

# Common rules
include common.mk
