ARCHS=i386 ppc
BUILD_HOST=${shell uname -p}-apple-darwin

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

define arch_template
_arch_${1}_%: ARCH=${1}
_arch_${1}_%: export CFLAGS:=${CFLAGS_${1}} ${ADDITIONAL_CFLAGS}
_arch_${1}_%: export LDFLAGS:=${LDFLAGS_${1}} ${ADDITIONAL_LDFLAGS}
_arch_${1}_%: export CXXFLAGS:=${CFLAGS_${1}} ${ADDITIONAL_CFLAGS}
_arch_${1}_%: ARCH_CONFIG:=${CONFIG_${1}}
_arch_${1}_%: %_${1};
endef

ifdef ARCH
arch_targets=_arch_${ARCH}_${1}
${eval ${call arch_template,${ARCH}}}
else
arch_targets=${foreach ARCH,${ARCHS},_arch_${ARCH}_${1}}
${foreach ARCH,${ARCHS},${eval ${call arch_template,${ARCH}}}}
endif

arch_test: ${call arch_targets,arch_test}
	@echo target: $@ dependencies: $^

arch_test_%:
	@echo target: $@ dependencies: $^ arch: ${ARCH}
	@echo CFLAGS: $$CFLAGS
	@echo LDFLAGS: $$LDFLAGS
	@echo CXXFLAGS: $$CXXFLAGS

