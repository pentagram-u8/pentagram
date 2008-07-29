TARGET_PREFIX=${realpath ./}
ROOT_DIRECTORY=${realpath ../../}

ADDITIONAL_CFLAGS=-I${TARGET_PREFIX}/${ARCH}/include
ADDITIONAL_LDFLAGS=-L${TARGET_PREFIX}/${ARCH}/lib

include ${TARGET_PREFIX}/archs.mk

DEPS=zlib libpng SDL SDL_ttf

all: extract configure build install
distclean: clean
	rm -fr ${ARCHS} build

define dep_template
_dep_${1}_%: DEP=${1}
_dep_${1}_%: %_${1};
endef

dep_targets=${foreach DEP,${DEPS},_dep_${DEP}_${1}}
${foreach DEP,${DEPS},${eval ${call dep_template,${DEP}}}}
${DEPS}: %: _dep_%_install;

extract: ${call dep_targets,extract};
configure: ${call dep_targets,configure};
build: ${call dep_targets,build};
install: ${call dep_targets,install};
clean: ${call dep_targets,clean};

extract_%: ${call arch_targets,.extract_%};
configure_%: extract_% ${call arch_targets,.configure_%};
build_%: configure_% ${call arch_targets,.build_%};
install_%: build_% ${call arch_targets,.install_%};
clean_%: ${call arch_targets,.clean_%};

# This seems to prevent my stub files from being deleted
.SECONDARY:

.extract_zlib_%: ${TARGET_PREFIX}/zlib-1.2.3.tar.gz
	-mkdir build
	-mkdir build/${ARCH}
	-rm -fr build/${ARCH}/${DEP}
	-mkdir build/${ARCH}/${DEP}
	cd build/${ARCH}/${DEP} && tar --strip-components=1 -xzf $<
	touch $@

.extract_libpng_%: ${TARGET_PREFIX}/libpng-1.2.29.tar.gz
	-mkdir build
	-mkdir build/${ARCH}
	-rm -fr build/${ARCH}/${DEP}
	-mkdir build/${ARCH}/${DEP}
	cd build/${ARCH}/${DEP} && tar --strip-components=1 -xzf $<
	touch $@

.extract_SDL_%: ${TARGET_PREFIX}/SDL-1.2.13.tar.gz
	-mkdir build
	-mkdir build/${ARCH}
	-rm -fr build/${ARCH}/${DEP}
	-mkdir build/${ARCH}/${DEP}
	cd build/${ARCH}/${DEP} && tar --strip-components=1 -xzf $<
	touch $@

.extract_SDL_ttf_%: ${TARGET_PREFIX}/SDL_ttf-2.0.9.tar.gz
	-mkdir build
	-mkdir build/${ARCH}
	-rm -fr build/${ARCH}/${DEP}
	-mkdir build/${ARCH}/${DEP}
	cd build/${ARCH}/${DEP} && tar --strip-components=1 -xzf $<
	touch $@

.configure_zlib_%:
	cd build/${ARCH}/${DEP} && ./configure --prefix=${TARGET_PREFIX}/${ARCH}
	touch $@

.configure_libpng_%: zlib
	cd build/${ARCH}/${DEP} && ./configure --prefix=${TARGET_PREFIX}/${ARCH} \
		${ARCH_CONFIG} --disable-dependency-tracking
	touch $@

.configure_SDL_%: libpng
	cd build/${ARCH}/${DEP} && ./configure --prefix=${TARGET_PREFIX}/${ARCH} \
		${ARCH_CONFIG} --disable-dependency-tracking --enable-video-x11=no \
		--enable-video-carbon=no --enable-video-cocoa=yes
	touch $@

.configure_SDL_ttf_%: SDL
	cd build/${ARCH}/${DEP} && ./configure --prefix=${TARGET_PREFIX}/${ARCH} \
		${ARCH_CONFIG} --disable-dependency-tracking
	touch $@

.build_%:
	cd build/${ARCH}/${DEP} && make
	@touch $@

.install_%:
	cd build/${ARCH}/${DEP} && make install
	@touch $@

.clean_%:
	rm -fr build/${ARCH}/${DEP}
	rm -fr .*_${DEP}_${ARCH}

