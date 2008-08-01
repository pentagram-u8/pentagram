ifndef ROOT_DIRECTORY
ROOT_DIRECTORY=${realpath ../../}
endif

include ${ROOT_DIRECTORY}/system/macosx/archs.mk

DEPS=zlib libpng SDL SDL_ttf

all: extract configure build install
distclean: clean
	rm -fr build

${foreach DEP,${DEPS},${eval ${call create_arch_targets,${DEP}}}}
${DEPS}: %: dep_install_%

extract: ${foreach DEP,${DEPS},dep_extract_${DEP}};
configure: ${foreach DEP,${DEPS},dep_configure_${DEP}};
build: ${foreach DEP,${DEPS},dep_build_${DEP}};
install: ${foreach DEP,${DEPS},dep_install_${DEP}};
clean: ${foreach DEP,${DEPS},dep_clean_${DEP}};

dep_extract_%: ${call arch_targets,%,extract};
dep_configure_%: dep_extract_% ${call arch_targets,%,configure};
dep_build_%: dep_configure_% ${call arch_targets,%,build};
dep_install_%: dep_build_% ${call arch_targets,%,install};
dep_clean_%: ${call arch_targets,%,clean};

# This seems to prevent my stub files from being deleted
.SECONDARY:

extract_zlib.%: ${ROOT_DIRECTORY}/zlib-1.2.3.tar.gz
	cd ${BUILD_DIR} && tar --strip-components=1 -xzf $<

extract_libpng.%: ${ROOT_DIRECTORY}/libpng-1.2.29.tar.gz
	cd ${BUILD_DIR} && tar --strip-components=1 -xzf $<

extract_SDL.%: ${ROOT_DIRECTORY}/SDL-1.2.13.tar.gz
	cd ${BUILD_DIR} && tar --strip-components=1 -xzf $<

extract_SDL_ttf.%: ${ROOT_DIRECTORY}/SDL_ttf-2.0.9.tar.gz
	cd ${BUILD_DIR} && tar --strip-components=1 -xzf $<

configure_zlib.%:
	cd ${BUILD_DIR} && ./configure --prefix=${PREFIX_DIR}

configure_libpng.%: zlib
	cd ${BUILD_DIR} && ./configure --prefix=${PREFIX_DIR} \
		${ARCH_CONFIG} --disable-dependency-tracking

configure_SDL.%: libpng
	cd ${BUILD_DIR} && ./configure --prefix=${PREFIX_DIR} \
		${ARCH_CONFIG} --disable-dependency-tracking --enable-video-x11=no \
		--enable-video-carbon=no --enable-video-cocoa=yes

configure_SDL_ttf.%: SDL
	cd ${BUILD_DIR} && ./configure --prefix=${PREFIX_DIR} \
		${ARCH_CONFIG} --disable-dependency-tracking

build_%:
	cd ${BUILD_DIR} && make

install_%:
	cd ${BUILD_DIR} && make install

clean_%:
	rm -fr ${BUILD_DIR}
	rm -fr ${ROOT_DIRECTORY}/build/${PROJECT}.build/${ARCH}_*.stamp 

