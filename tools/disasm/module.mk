# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to now our path anyway.
LPATH := tools/disasm
CXXFLAGS += -I../../..

LSRC := $(wildcard $(LPATH)/*.cc)
LPRODUCTS := disasm

disasm_OBJ = ../../misc/Args.o Disasm.o

# Common rules
include common.mk
