# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to now our path anyway.
LPATH := tools/fold

LSRC := $(wildcard $(LPATH)/*.cpp)
LPRODUCTS := fold

fold_OBJ = \
	$(ARGS) \
	$(MISC) \
	$(CONF) \
	$(KERNEL) \
	$(GAMES) \
	filesys/FileSystem.o \
	filesys/Flex.o \
	tools/fold/Fold.o \
	tools/fold/Folder.o \
	tools/fold/Type.o \
	tools/fold/IfNode.o \
	tools/fold/OperatorNodes.o \
	tools/fold/CallNodes.o \
	tools/fold/VarNodes.o \
	tools/fold/FuncNodes.o \
	tools/fold/LoopScriptNodes.o

# Common rules
include $(srcdir)/common.mk
