# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/disasm
CXXFLAGS += -I../../..

LSRC := $(wildcard $(LPATH)/*.cc)
LPRODUCTS := disasm

disasm_OBJ = ../../misc/Args.o ../../filesys/FileSystem.o Disasm.o ../../misc/Console.o ../../misc/Q_strcasecmp.o
# ../../filesys/Flex.o
# Common rules
include common.mk
