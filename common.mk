# This file is included by all module.mk files, and provides common functionality
# You normally should never have to edit it.

# Generate .rules files for each GUI application target
LRULES := $(patsubst %,$(LPATH)/$(DEPDIR)/%.rules,$(LGUIPRODUCTS))
$(LRULES): system/auto/genrules.pl
	-$(MKDIR) $(dir $@)
	$(top_srcdir)/system/auto/genrules.pl --gui $(notdir $(basename $@)) > $@
-include $(LRULES) $(EMPTY_FILE)


# Generate .rules files for each console application target
LRULES := $(patsubst %,$(LPATH)/$(DEPDIR)/%.rules,$(LCONPRODUCTS))
$(LRULES): system/auto/genrules.pl
	-$(MKDIR) $(dir $@)
	$(top_srcdir)/system/auto/genrules.pl --con $(notdir $(basename $@)) > $@
-include $(LRULES) $(EMPTY_FILE)

LPRODUCTS := $(LGUIPRODUCTS) $(LCONPRODUCTS)

# include generated dependencies (we append EMPTY_FILE to avoid warnings if
# the list happens to be empty)
-include $(wildcard $(LPATH)/$(DEPDIR)/*.d) $(EMPTY_FILE)

# Transform LSRC to LOBJ (change each .cpp into .o, in the builddir)
# (two steps to make sure no .cpp files will be left in LOBJ for make clean)
LOBJ := $(patsubst $(srcdir)/%,$(top_builddir)/%,$(patsubst %.cpp,%.o,$(filter %.cpp,$(LSRC))) $(patsubst %.m,%.o,$(filter %.m,$(LSRC))))

# Transform LPRODUCTS to have full path information
LPRODUCTS := $(patsubst %,$(LPATH)/%$(EXEEXT),$(LPRODUCTS))

# Local all target
all-$(LPATH): $(LPRODUCTS) $(LOBJ)

# We want that "make dirname" works just like "make all-dirname"
$(LPATH): all-$(LPATH)

# Local clean target
LOBJ-$(LPATH) := $(LOBJ) 
LPRODUCTS-$(LPATH) := $(LPRODUCTS)
clean-$(LPATH): clean-% :
	-$(RM) $(LOBJ-$*) $(LPRODUCTS-$*)

# The global all/clean targets should invoke all sub-targets, we do that here
all: all-$(LPATH)
clean: clean-$(LPATH)


# reset variables

LSRC :=
LGUIPRODUCTS :=
LCONPRODUCTS :=


.PHONY: $(LPATH) all-$(LPATH) clean-$(LPATH)
