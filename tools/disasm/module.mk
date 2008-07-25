# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/disasm

LSRC := $(wildcard $(srcdir)/$(LPATH)/*.cpp)
LCONPRODUCTS := disasm

include $(srcdir)/objects.mk

disasm_OBJ = \
	$(ARGS) \
	$(MISC) \
	$(CONF) \
	$(GAMES) \
	$(FILESYS) \
	$(SYSTEM) \
	kernel/CoreApp.o \
	tools/disasm/Disasm.o \
	tools/fold/Type.o \
	tools/fold/IfNode.o \
	tools/fold/OperatorNodes.o \
	tools/fold/Folder.o \
	tools/fold/CallNodes.o \
	tools/fold/VarNodes.o \
	tools/fold/FuncNodes.o \
	tools/fold/LoopScriptNodes.o

# Common rules
include $(srcdir)/common.mk
