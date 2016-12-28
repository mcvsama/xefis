# vim:ts=4

first: all

# Override these variables in Makefile.local.

LC_ALL			:= C
export LC_ALL

# Show verbose compilation commands?
VERBOSE			:= 0
# How many threads use for compilation?
THREADS			:= $(shell grep processor /proc/cpuinfo|wc -l)
# Architecture:
ARCH			:= $(shell uname -m)
# Enable profiling?
PROFILING		:= 0
# Debugging enabled?
DEBUG			:= 0
# Where Qt is installed?
QT_PREFIX		:= /usr
# Profile name:
PROFILE			:=
# Version:
VERSION			:= alpha0.1dev
# Release version? (no asserts, etc).
RELEASE			:= 0
# Set to 1 to disable some UB-generating optimizations:
UB_OPTS_DISABLE	:= 0

# Predefined profiles:
ifeq ($(PROFILE),rpi)
CXXFLAGS += -finline -marm -mfpu=vfp -mtune=arm1176jzf-s -march=armv6zk -mfloat-abi=hard -falign-functions=4 -funroll-loops -fomit-frame-pointer
endif

# Undefined-Behaviour handling:
ifeq ($(UB_OPTS_DISABLE),1)
CXXFLAGS += -fno-strict-overflow -fno-delete-null-pointer-checks -fno-strict-aliasing -fsanitize=undefined
endif

ifeq ($(PROFILE),bbb)
CXXFLAGS += -finline -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=hard -falign-functions=4 -funroll-loops -fomit-frame-pointer
endif

CXX				:= g++
DEPCC			:= $(CXX)
ifeq ($(CXX),g++)
CXXFLAGS		+= -fPIC -O4 -g -c -std=c++17 -Wall -Wall -Wextra -Wunused -Wunused-function -Wunused-label -Wnoexcept -fstrict-aliasing -Wstrict-aliasing=3 -fnothrow-opt -pedantic-errors
CXXFLAGS		+= -pthread -rdynamic -I. -Ilib
endif
ifeq ($(CXX),clang++)
CLANG_DISABLED_WARNINGS = mismatched-tags disabled-macro-expansion c++98-compat-pedantic documentation-unknown-command
CXXFLAGS		+= -fPIE -O3 -g -c -std=c++17 -Wall -Wall -Wextra -Wunused -Wunused-function -Wunused-label -fstrict-aliasing $(foreach warning, $(CLANG_DISABLED_WARNINGS), -Wno-$(warning))
CXXFLAGS		+= -pthread -I. -Ilib
endif
CXXFLAGS		+= -finline -funroll-loops -fomit-frame-pointer
SO				:= $(CXX) -shared
SOFLAGS			:=
LD				:= $(CXX)
LDFLAGS			:= -rdynamic
AR				:= ar
MOC				:= $(QT_PREFIX)/bin/moc-qt5
LIBS			+= m boost_system pthread mhash
PKGCONFIGS		+= Qt5Core Qt5Gui Qt5Network Qt5Xml Qt5Widgets zlib
CXXFLAGS_s		:= $(CXXFLAGS)
ifeq ($(DEBUG),1)
XEFIS_FEATURES	+= XEFIS_ENABLE_ASSERT
XEFIS_FEATURES	+= XEFIS_ENABLE_FATAL_ASSERT
CXXFLAGS		+= -O2 -ggdb3 -fno-inline -fno-eliminate-unused-debug-types -D_GLIBCXX_DEBUG
else
CXXFLAGS		+= -DQT_NO_DEBUG
endif
CXXFLAGS		+= -DCXXFLAGS='"$(CXXFLAGS_s)"' $(foreach feature, $(XEFIS_FEATURES), -D$(feature))
LDFLAGS			+= $(foreach lib, $(LIBS), -l$(lib))
LDFLAGS			+= $(shell pkg-config --libs $(PKGCONFIGS))
CXXFLAGS		+= $(shell pkg-config --cflags $(PKGCONFIGS))

.PHONY: first all dep help clean distclean release doc check

HEADERS =
SOURCES =
OBJECTS =
NODEP_OBJECTS =
MOCSRCS =
TARGETS =

#### Helpers ####

ifeq ($(shell tput colors), -1)
	_s = ""
	_l = ""
else
	_s = "[1;34m"
	_l = "[0m"
endif

#### Makefiles ####

-include Makefile.local

ifneq ($(VERBOSE),1)
MAKEFLAGS += --silent
endif

ifneq ($(THREADS),)
MAKEFLAGS += -j$(THREADS)
endif

ifeq ($(PROFILING),1)
CXXFLAGS += -pg -fno-omit-frame-pointer
LDFLAGS += -pg
endif

ifeq ($(PROFILING),1)
PROFILING_s = -profiling
else
PROFILING_s =
endif

ifeq ($(DEBUG),1)
DEBUG_s = -debug
else
DEBUG_s =
endif

ifeq ($(RELEASE),1)
CXXFLAGS += -DBOOST_DISABLE_ASSERTS -DNDEBUG
else
CXXFLAGS += -DXEFIS_ENABLE_ASSERT
endif

ifneq ($(PROFILE),)
PROFILE_s = -$(PROFILE)
else
PROFILE_s =
endif

dist			= $(ARCH)$(PROFILING_s)$(DEBUG_s)$(PROFILE_s)
distdir			:= build/$(dist)
depsdir			= $(distdir)/deps
objsdir			= $(distdir)/objects
mocsdir			= $(distdir)/mocs
reldir			= release/$(dist)
relname			= xefis-$(VERSION)-$(dist)
reldir_pkg		= $(reldir)/$(relname)
prepdir			= $(shell mkdir -p "$$(dirname "$(strip $(1))")")
mkmocs			= $(foreach file, $(1), $(mocsdir)/$(dir $(file))$(shell basename '$(file)' $(suffix $(file))).moc.cc)
mkdeps			= $(foreach file, $(1), $(depsdir)/$(dir $(file))$(shell basename '$(file)' $(suffix $(file))).dep)
mkobjs			= $(foreach file, $(1), $(objsdir)/$(dir $(file))$(shell basename '$(file)' $(suffix $(file))).o)
mkmocobjs		= $(foreach file, $(1), $(dir $(file))$(shell basename '$(file)' $(suffix $(file))).o)

include configs/Makefile.configs
include $(foreach config, $(XEFIS_CONFIGS), configs/$(config)/Makefile.sources)
include Makefile.sources

#### Commit ID support ####

ifneq ($(VERSION_FILE),)
GIT_AVAILABLE := $(shell which git 1>/dev/null && echo OK)

ifeq ($(GIT_AVAILABLE),OK)
COMMIT			= $(shell cat .commit 2>/dev/null)
COMMIT_REAL		= $(shell git rev-parse HEAD 2>/dev/null)
BRANCH			= $(shell cat .branch 2>/dev/null)
BRANCH_REAL		= $(shell git branch --no-color 2>/dev/null | sed -e '/^[^*]/d' -e 's/* \(.*\)/\1/')

ifeq ($(COMMIT),$(COMMIT_REAL))
COMMIT_FILE		=
else
COMMIT_FILE		= .commit
endif

ifeq ($(BRANCH),$(BRANCH_REAL))
BRANCH_FILE		=
else
BRANCH_FILE		= .branch
endif

.PHONY: .commit .branch

.commit:
	@if [ "$(COMMIT)" != "$(COMMIT_REAL)" ]; then \
		echo " Saving commit ID"; \
		echo "$(COMMIT_REAL)" >$@; \
	 fi

.branch:
	@if [ "$(BRANCH)" != "$(BRANCH_REAL)" ]; then \
		echo " Saving branch name"; \
		echo "$(BRANCH_REAL)" >$@; \
	 fi

$(VERSION_FILE): $(COMMIT_FILE) $(BRANCH_FILE)
	@echo "// Do not edit - automatically generated." >$@
	@echo "namespace xf { namespace version {" >>$@
	@echo "	const char* commit = \"$(COMMIT_REAL)\";" >>$@
	@echo "	const char* branch = \"$(BRANCH_REAL)\";" >>$@
	@echo "	const char* version = \"$(VERSION)\";" >>$@
	@echo "} }" >>$@

$(call mkobjs, $(VERSION_FILE)): $(VERSION_FILE)
endif # GIT_AVAILABLE == OK
endif # VERSION_FILE != ""

#### Rules ####

DEPFILES := $(call mkdeps, $(subst $(VERSION_FILE),,$(SOURCES)))
MAINDEPFILE := $(depsdir)/Makefile.dep

all: $(MAINDEPFILE) $(DEPFILES) $(TARGETS)

dep: $(DEPFILES)

help:
	@echo 'Available targets:'
	@echo '  all        Compiles program.'
	@echo '  dep        Generates dependencies.'
	@echo '  clean      Cleans source tree and dep files'
	@echo '  distclean  Cleans build directory.'
	@echo '  release    Creates release.'
	@echo '  help       Shows this help.'

clean:
	@rm -f $(MOCSRCS)
	@rm -f $(OBJECTS)
	@rm -f $(MOCOBJS)
	@rm -f $(DEPFILES)
	@rm -f $(MAINDEPFILE)

distclean:
	@rm -rf build

release:
	@echo Unimplemented

doc:
	@cd doc && doxygen doxygen-conf/doxygen.conf

check:
	cppcheck --force --std=c++17 --language=c++ --inconclusive --enable=all --template "{id}:{file}:{line} ({severity}): {message}" --suppressions-list=cppcheck.suppressions \
		lib modules xefis $(HEADERS) 2> check.log

$(MAINDEPFILE): Makefile.sources $(DEPFILES)
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	@rm -f $@
	@echo "# DO NOT EDIT!!! (all changed will be lost!)" >> $@
	@echo "# This file has been generated automatically by Makefile from source files." >> $@
	@for file in $(DEPFILES); do \
		cat "$$file" >> $@; \
	 done;
	@for file in $(MOCSRCS); do \
		echo "$$(dirname "$$file")/$$(basename "$$file" .cc).o: $$file" >> $@; \
	 done;

$(depsdir)/%.dep: %.cc %.h
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	$(DEPCC) $(CXXFLAGS) -MM -MQ "$(objsdir)/$$(dirname "$<")/$$(basename "$<" $$(expr "$<" : '.*\(\..*\)$$')).o" $< > $@

$(depsdir)/%.dep: %.cc
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	$(DEPCC) $(CXXFLAGS) -MM -MQ "$(objsdir)/$$(dirname "$<")/$$(basename "$<" $$(expr "$<" : '.*\(\..*\)$$')).o" $< > $@

$(depsdir)/%.dep: %.h
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	$(DEPCC) $(CXXFLAGS) -MM -MQ "$(objsdir)/$$(dirname "$<")/$$(basename "$<" $$(expr "$<" : '.*\(\..*\)$$')).o" $< > $@

$(depsdir)/%.moc.dep: %.moc.cc
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	$(DEPCC) $(CXXFLAGS) -MM -MQ "$$(dirname "$<")/$$(basename "$<" $$(expr "$<" : '.*\(\..*\)$$')).o" $< > $@

%.o:
	$(call prepdir, $@)
	@echo $(_s) "C++     " $(_l) $@
	$(CXX) $(CXXFLAGS) $< -o $@

%.a:
	$(call prepdir, $@)
	@echo $(_s) "AR      " $(_l) $@
	@rm -f $@
	$(AR) cr $@ $^

%.so:
	$(call prepdir, $@)
	@echo $(_s) "LD [M]  " $(_l) $@
	$(SO) $(SOFLAGS) $^ -o $@

$(mocsdir)/%.moc.cc: %.h
	$(call prepdir, $@)
	@echo $(_s) "MOC     " $(_l) $@
	$(MOC) $< -o $@

$(LINKEDS):
	$(call prepdir, $@)
	@echo $(_s) "LD      " $(_l) $@
	$(LD) -o $@ $^ $(LDFLAGS)

-include $(MAINDEPFILE)
ifneq ($(DIST),)
	include Makefile.dist.$(DIST)
endif

