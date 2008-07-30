ARCHS=i386 ppc
BUILD_HOST=${shell uname -p}-apple-darwin
TARGET_PREFIX=${realpath ./}

CFLAGS_i386=-O2 -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -arch i386
LDFLAGS_i386=-arch i386
CONFIG_i386=--build=${BUILD_HOST} --host=i386-apple-darwin

CFLAGS_ppc=-O2 -isysroot /Developer/SDKs/MacOSX10.3.9.sdk -mmacosx-version-min=10.3 -arch ppc
LDFLAGS_ppc=-arch ppc
CONFIG_ppc=--build=${BUILD_HOST} --host=powerpc-apple-darwin

# These are off for the moment
# 10.5 required to link Foundation in 64-bit
CFLAGS_x86_64=-O2 -isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -arch x86_64
LDFLAGS_x86_64=-arch x86_64
CONFIG_x86_64=--build=${BUILD_HOST} --host=i386-apple-darwin

CFLAGS_ppc64=-O2 -isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -arch ppc64
LDFLAGS_ppc64=-arch ppc64
CONFIG_ppc64=--build=${BUILD_HOST} --host=powerpc-apple-darwin


# Weird thing: if the stamp is foo_${2}.stamp, we won't match here due to the
#  matches to foo_% - see make manual 10.8 Implicit Rule Search Algorithm

define arch_template
build/${1}.build/${2}_%.stamp: ARCH=${2}
build/${1}.build/${2}_%.stamp: PROJECT=${1}
build/${1}.build/${2}_%.stamp: PREFIX_DIR=${TARGET_PREFIX}/build/${2}
build/${1}.build/${2}_%.stamp: BUILD_DIR=${TARGET_PREFIX}/build/${1}.build/${2}
build/${1}.build/${2}_%.stamp: export CFLAGS:=${CFLAGS_${2}} -I${TARGET_PREFIX}/build/${2}/include
build/${1}.build/${2}_%.stamp: export LDFLAGS:=${LDFLAGS_${2}} -L${TARGET_PREFIX}/build/${2}/lib
build/${1}.build/${2}_%.stamp: export CXXFLAGS:=${CFLAGS_${2}} -I${TARGET_PREFIX}/build/${2}/include
build/${1}.build/${2}_%.stamp: ARCH_CONFIG:=${CONFIG_${2}}
build/${1}.build/${2}_%.stamp: build/${1}.build/${2} %_${1}.${2}
	@touch $$@

build/${1}.build/${2}:
	@-mkdir build
	@-mkdir build/${1}.build
	@-mkdir build/${1}.build/${2}
endef

ifdef ARCH
arch_targets=build/${1}.build/${ARCH}_${2}.stamp
create_arch_targets=${eval ${call arch_template,${1},${ARCH}}}
else
arch_targets=${foreach ARCH,${ARCHS},build/${1}.build/${ARCH}_${2}.stamp}
create_arch_targets=${foreach ARCH,${ARCHS},${eval ${call arch_template,${1},${ARCH}}}}
endif

#${call create_arch_targets,test};
#arch_test: ${call arch_targets,test,arch_test}
#	@echo target: $@ dependencies: $^

#arch_test_%:
#	@echo target: $@ dependencies: $^ arch: ${ARCH}
#	@echo CFLAGS: $$CFLAGS
#	@echo LDFLAGS: $$LDFLAGS
#	@echo CXXFLAGS: $$CXXFLAGS
