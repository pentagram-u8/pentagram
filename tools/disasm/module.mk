# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/disasm

LSRC := $(wildcard $(LPATH)/*.cpp)
LPRODUCTS := disasm

disasm_OBJ = ../../misc/Args.o ../../filesys/FileSystem.o Disasm.o ../../misc/Console.o ../../misc/Q_strcasecmp.o
# ../../filesys/Flex.o
# Common rules
include common.mk
