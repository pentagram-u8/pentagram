ifndef ROOT_DIRECTORY
ROOT_DIRECTORY=${realpath ../../}
endif

include ${ROOT_DIRECTORY}/system/macosx/archs.mk

DEPS=zlib libpng SDL freetype SDL_ttf

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

${ROOT_DIRECTORY}/build/zlib.tar.gz:
	curl -o $@ -L -S http://www.zlib.net/zlib-1.2.3.tar.gz

${ROOT_DIRECTORY}/build/libpng.tar.gz:
	curl -o $@ -L -S http://prdownloads.sourceforge.net/libpng/libpng-1.2.41.tar.gz

${ROOT_DIRECTORY}/build/SDL.tar.gz:
	curl -o $@ -L -S http://libsdl.org/release/SDL-1.2.14.tar.gz

${ROOT_DIRECTORY}/build/freetype.tar.gz:
	curl -o $@ -L -S http://prdownloads.sourceforge.net/freetype/freetype-2.3.7.tar.gz

${ROOT_DIRECTORY}/build/SDL_ttf.tar.gz:
	curl -o $@ -L -S http://www.libsdl.org/projects/SDL_ttf/release/SDL_ttf-2.0.9.tar.gz

extract_zlib.%: ${ROOT_DIRECTORY}/build/zlib.tar.gz
	cd ${BUILD_DIR} && tar --strip-components=1 -xzf $<

extract_libpng.%: ${ROOT_DIRECTORY}/build/libpng.tar.gz
	cd ${BUILD_DIR} && tar --strip-components=1 -xzf $<

extract_SDL.%: ${ROOT_DIRECTORY}/build/SDL.tar.gz
	cd ${BUILD_DIR} && tar --strip-components=1 -xzf $<

extract_freetype.%: ${ROOT_DIRECTORY}/build/freetype.tar.gz
	cd ${BUILD_DIR} && tar --strip-components=1 -xzf $<

extract_SDL_ttf.%: ${ROOT_DIRECTORY}/build/SDL_ttf.tar.gz
	cd ${BUILD_DIR} && tar --strip-components=1 -xzf $<

configure_zlib.%:
	cd ${BUILD_DIR} && LDSHARED="${CC} ${LDFLAGS} -dynamiclib \
		-install_name ${PREFIX_DIR}/lib/libz.1.dylib \
		-compatibility_version 1 -current_version 1.2.3" \
		./configure --prefix=${PREFIX_DIR} --shared

configure_libpng.%: zlib
	cd ${BUILD_DIR} && ./configure --prefix=${PREFIX_DIR} \
		${ARCH_CONFIG} --disable-dependency-tracking

configure_SDL.%: libpng
	cd ${BUILD_DIR} && ./configure --prefix=${PREFIX_DIR} \
		${ARCH_CONFIG} --disable-dependency-tracking --enable-video-x11=no \
		--enable-video-carbon=no --enable-video-cocoa=yes

# Freetype's configure scripts attempt to figure out the cross compiler on
# their own when the build != host instead of trusting ${CC}.
configure_freetype.%: zlib
	cd ${BUILD_DIR} && CC_BUILD="${CC}" \
		./configure --prefix=${PREFIX_DIR} \
		${ARCH_CONFIG} --with-old-mac-fonts

configure_SDL_ttf.%: SDL freetype
	cd ${BUILD_DIR} && ./configure --prefix=${PREFIX_DIR} \
		--with-freetype-prefix=${PREFIX_DIR} \
		${ARCH_CONFIG} --disable-dependency-tracking

build_%:
	cd ${BUILD_DIR} && make

install_%:
	cd ${BUILD_DIR} && make install

clean_%:
	rm -fr ${BUILD_DIR}
	rm -fr ${ROOT_DIRECTORY}/build/${PROJECT}.build/${ARCH}_*.stamp 

