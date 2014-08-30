# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := .

LSRC := $(wildcard $(srcdir)/$(LPATH)/*.cpp)
LGUIPRODUCTS := pentagram
#LCONPRODUCTS := llc

pentagram_OBJ = \
	$(KERNEL) \
	$(USECODE) \
	$(FILESYS) \
	$(GAMES) \
	$(GAMES2) \
	$(GRAPHICS) \
	$(SCALERS) \
	$(FONTS) \
	$(MISC) \
	$(CONVERT) \
	$(CONF) \
	$(GUMPS) \
	$(WIDGETS) \
	$(WORLD) \
	$(ACTORS) \
	$(ARGS) \
	$(AUDIO) \
	$(MIDI) \
	$(TIMIDITY) \
	$(SYSTEM) \
	filesys/OutputLogger.o \
	kernel/GUIApp.o \
	misc/version.o \
	pentagram.o

llc_OBJ = \
	kernel/Kernel.o \
	kernel/Process.o \
	$(FILESYS) \
	$(GAMES) \
	$(MISC) \
	$(CONF) \
	$(COMPILE) \
	$(DISASM) \
	$(ARGS) \
	kernel/CoreApp.o \
	kernel/ConApp.o \
	tools/compile/llc.o

# make version.o depend on everything to force a rebuild (for build time/date)
misc/version.o: $(filter-out misc/version.o,$(pentagram_OBJ))


# install rules

install-bin: pentagram$(EXEEXT)
	$(INSTALL) -d "$(DESTDIR)$(bindir)"
	$(INSTALL_PROGRAM) "$(top_builddir)/pentagram$(EXEEXT)" "$(DESTDIR)$(bindir)/pentagram$(EXEEXT)"

ifeq ($(USE_BUILTIN_DATA),yes)
install-data:
else
install-data: $(DATA_FILES)
	$(INSTALL) -d "$(DESTDIR)$(datapath)"
	$(INSTALL_DATA) $(patsubst %,$(top_srcdir)/%,$(DATA_FILES)) "$(DESTDIR)$(datapath)"
endif

install: install-bin install-data


# Common rules
include $(srcdir)/common.mk

# Mac OS X rules
ifeq ($(HOST_SYSTEM),MACOSX)

bundle_name = pentagram.app

bundle: pentagram$(EXEEXT)
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources/
	mkdir -p $(bundle_name)/Contents/Documents
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	$(INSTALL_DATA) system/macosx/Info.plist $(bundle_name)/Contents/
	$(INSTALL_PROGRAM) pentagram$(EXEEXT) $(bundle_name)/Contents/MacOS/
	$(INSTALL_DATA) $(top_srcdir)/system/desktop/pentagram.* $(bundle_name)/Contents/Resources/
	$(INSTALL_DATA) $(top_srcdir)/data/*.ini $(bundle_name)/Contents/Resources/
	$(INSTALL_DATA) $(top_srcdir)/data/*.tga $(bundle_name)/Contents/Resources/
	$(INSTALL_DATA) $(top_srcdir)/data/*.png $(bundle_name)/Contents/Resources/
	$(INSTALL_DATA) $(top_srcdir)/data/*.ttf $(bundle_name)/Contents/Resources/
	$(INSTALL_DATA) $(top_srcdir)/data/*.txt $(bundle_name)/Contents/Resources/
	$(INSTALL_DATA) $(top_srcdir)/AUTHORS $(bundle_name)/Contents/Documents
	$(INSTALL_DATA) $(top_srcdir)/COPYING $(bundle_name)/Contents/Documents
	$(INSTALL_DATA) $(top_srcdir)/FAQ $(bundle_name)/Contents/Documents
	$(INSTALL_DATA) $(top_srcdir)/README $(bundle_name)/Contents/Documents
ifneq ($(OSX_CODE_SIGNATURE),)
	codesign -f -s "$(OSX_CODE_SIGNATURE)" $(bundle_name)
endif
	
install-exec-local: bundle
	mkdir -p $(DESTDIR)/Applications/
	cp -r $(bundle_name) $(DESTDIR)/Applications/

image: bundle
	mkdir pentagram-snapshot
	cp $(top_srcdir)/AUTHORS ./pentagram-snapshot/Authors
	cp $(top_srcdir)/COPYING ./pentagram-snapshot/License
	cp $(top_srcdir)/README ./pentagram-snapshot/ReadMe
	cp $(top_srcdir)/FAQ ./pentagram-snapshot/FAQ
	SetFile -t ttro -c ttxt ./pentagram-snapshot/*
	CpMac -r $(bundle_name) ./pentagram-snapshot/
	hdiutil create -ov -format UDZO -imagekey zlib-level=9 -fs HFS+ \
					-srcfolder pentagram-snapshot \
					-volname "Pentagram snapshot$(REVISION)" \
					Pentagram.dmg
	rm -rf pentagram-snapshot

clean-local:
	-rm -f Pentagram.dmg
	-rm -rf $(bundle_name)

endif
	
.PHONY: install install-bin install-data
