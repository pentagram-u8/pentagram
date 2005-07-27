# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := gumps/widgets

LSRC := $(wildcard $(srcdir)/$(LPATH)/*.cpp)

# Common rules
include $(srcdir)/common.mk
