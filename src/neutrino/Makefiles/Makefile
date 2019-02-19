# vim:ts=4

first: all

LC_ALL := C
export LC_ALL

# Override these variables in Makefile.local.

MAKE_CONFIG_PROJECT_NAME		:= project
# Show verbose compilation commands?
MAKE_CONFIG_VERBOSE				:= 0
# How many threads use for compilation?
MAKE_CONFIG_THREADS				:= $(shell nproc)
# Architecture:
MAKE_CONFIG_ARCH				:= $(shell uname -m)
# Enable profiling?
MAKE_CONFIG_PROFILING			:= 0
# Debugging enabled?
MAKE_CONFIG_DEBUG				:= 0
# Where Qt is installed?
MAKE_CONFIG_QT_PREFIX			:= /usr/local
# Profile name:
MAKE_CONFIG_PROFILE				?= release
# Version:
MAKE_CONFIG_VERSION				:= alpha0.1dev
# Release version? (no asserts, etc).
MAKE_CONFIG_RELEASE				:= 0
# Set to 1 to disable some UB-generating optimizations:
MAKE_CONFIG_UB_OPTS_DISABLE		:= 0
# Sanitizers?
MAKE_CONFIG_SANITIZERS			:=
MAKE_CONFIG_NO_SANITIZE			:=
# Coverage?
MAKE_CONFIG_COVERAGE			:= 0

CXX				:= g++
DEPCC			:= $(CXX)
ifeq ($(CXX),g++)
CXXFLAGS		+= -fPIC -g -c -std=c++17 -Wall -Wall -Wextra -Wunused -Wunused-function -Wunused-label -Wnoexcept -fstrict-aliasing -Wstrict-aliasing=3 -fconcepts -fnothrow-opt -pedantic-errors -D_GLIBCXX_USE_CXX11_ABI=1 -Wshadow-compatible-local
CXXFLAGS		+= -pthread -rdynamic -I. -Ilib
endif
ifeq ($(CXX),clang++)
CLANG_DISABLED_WARNINGS = mismatched-tags disabled-macro-expansion c++98-compat-pedantic documentation-unknown-command
CXXFLAGS		+= -fPIE -g -c -std=c++17 -Wall -Wall -Wextra -Wunused -Wunused-function -Wunused-label -fconcepts -fstrict-aliasing $(foreach warning, $(CLANG_DISABLED_WARNINGS), -Wno-$(warning))
CXXFLAGS		+= -pthread -I. -Ilib
endif
CXXFLAGS		+= -finline -funroll-loops -fomit-frame-pointer
SO				:= $(CXX) -shared
SOFLAGS			+=
LD				:= $(CXX)
LDFLAGS			+= -rdynamic
AR				:= ar
MOC				:= $(MAKE_CONFIG_QT_PREFIX)/bin/moc-qt5
LIBS			+= m boost_system boost_filesystem pthread mhash X11
PKGCONFIGS		+= Qt5Core Qt5Gui Qt5Network Qt5Xml Qt5Widgets Qt5X11Extras zlib
CXXFLAGS_s		:= $(CXXFLAGS)
CXXFLAGS		+= -DCXXFLAGS='"$(CXXFLAGS_s)"'
LDFLAGS			+= $(foreach lib, $(LIBS), -l$(lib))
LDFLAGS			+= $(shell pkg-config --libs $(PKGCONFIGS))
# This subst-hack is to hide warnings from Qt that happen from time to time and obscure compiler's output:
CXXFLAGS		+= $(subst -I,-isystem ,$(shell pkg-config --cflags $(PKGCONFIGS)))

.PHONY: first all dep help clean depclean distclean release doc check

HEADERS =
SOURCES =
OBJECTS =
NODEP_OBJECTS =
SOURCES_MOC =
TARGETS =

################ Helpers ################

ifeq ($(shell tput colors), -1)
	_s = ""
	_l = ""
else
	_s = "[1;34m"
	_l = "[0m"
endif

################ Makefiles ################

-include Makefile.local
-include Makefile.local.$(MAKE_CONFIG_PROFILE)

Makefile.local:
	false

Makefile.local.$(MAKE_CONFIG_PROFILE):
	false

ifeq ($(MAKE_CONFIG_DEBUG),1)
CXXFLAGS += -O0 -ggdb3 -fno-inline -fno-eliminate-unused-debug-types -D_GLIBCXX_DEBUG
else
CXXFLAGS += -O4 -DQT_NO_DEBUG
endif

ifneq ($(MAKE_CONFIG_VERBOSE),1)
MAKEFLAGS += --silent
endif

ifneq ($(MAKE_CONFIG_THREADS),)
MAKEFLAGS += -j$(MAKE_CONFIG_THREADS)
endif

ifeq ($(MAKE_CONFIG_PROFILING),1)
CXXFLAGS += -pg -fno-omit-frame-pointer
LDFLAGS += -pg
endif

ifeq ($(MAKE_CONFIG_RELEASE),1)
CXXFLAGS += -DBOOST_DISABLE_ASSERTS -DNDEBUG
endif

ifneq ($(MAKE_CONFIG_PROFILE),)
PROFILE_s = -$(MAKE_CONFIG_PROFILE)
else
PROFILE_s =
endif

# Undefined-Behaviour handling:
ifeq ($(MAKE_CONFIG_UB_OPTS_DISABLE),1)
CXXFLAGS += -fno-strict-overflow -fno-delete-null-pointer-checks -fno-strict-aliasing
endif

ifneq ($(MAKE_CONFIG_SANITIZERS),)
CXXFLAGS += $(foreach sanitizer, $(MAKE_CONFIG_SANITIZERS), -fsanitize=$(sanitizer))
LDFLAGS += $(foreach sanitizer, $(MAKE_CONFIG_SANITIZERS), -fsanitize=$(sanitizer))
endif

ifneq ($(MAKE_CONFIG_NO_SANITIZE),)
CXXFLAGS += $(foreach sanitizer, $(MAKE_CONFIG_NO_SANITIZE), -fno-sanitize=$(sanitizer))
LDFLAGS += $(foreach sanitizer, $(MAKE_CONFIG_NO_SANITIZE), -fno-sanitize=$(sanitizer))
endif

ifeq ($(MAKE_CONFIG_COVERAGE),1)
CXXFLAGS += -fprofile-arcs -ftest-coverage
LDFLAGS += -fprofile-arcs -ftest-coverage
endif

dist			= $(MAKE_CONFIG_ARCH)$(PROFILE_s)
distdir			:= build/$(dist)
depsdir			= $(distdir)/deps
objsdir			= $(distdir)/objects
mocsdir			= $(distdir)/mocs
reldir			= release/$(dist)
relname			= $(MAKE_CONFIG_PROJECT_NAME)-$(MAKE_CONFIG_VERSION)-$(dist)
reldir_pkg		= $(reldir)/$(relname)
prepdir			= $(shell mkdir -p "$$(dirname "$(strip $(1))")")
mkmocs			= $(foreach file, $(1), $(mocsdir)/$(dir $(file))$(shell basename '$(file)' $(suffix $(file))).moc.cc)
mkdeps			= $(foreach file, $(1), $(depsdir)/$(dir $(file))$(shell basename '$(file)' $(suffix $(file))).dep)
mkobjs			= $(foreach file, $(1), $(objsdir)/$(dir $(file))$(shell basename '$(file)' $(suffix $(file))).o)
mkmocobjs		= $(foreach file, $(1), $(dir $(file))$(shell basename '$(file)' $(suffix $(file))).o)

include Makefile.sources.neutrino
include Makefile.sources

################ Commit ID support ################

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
	@echo "namespace neutrino::version {" >>$@
	@echo "	const char* commit = \"$(COMMIT_REAL)\";" >>$@
	@echo "	const char* branch = \"$(BRANCH_REAL)\";" >>$@
	@echo "	const char* version = \"$(MAKE_CONFIG_VERSION)\";" >>$@
	@echo "}" >>$@

$(call mkobjs, $(VERSION_FILE)): $(VERSION_FILE)
endif # GIT_AVAILABLE == OK
endif # VERSION_FILE != ""

################ Rules ################

DEPFILES := $(call mkdeps, $(subst $(VERSION_FILE),,$(SOURCES)))
MAINDEPFILE := $(depsdir)/Makefile.dep

all: $(MAINDEPFILE) $(DEPFILES) $(TARGETS)

dep: $(DEPFILES)

help:
	@echo 'Available targets:'
	@echo '  all        Compile program.'
	@echo '  dep        Generate dependencies.'
	@echo '  clean      Clean source tree and dep files'
	@echo '  depclean   Clean dependency files'
	@echo '  distclean  Clean build directory.'
	@echo '  release    Create release.'
	@echo '  help       Show this help.'

clean:
	@rm -f $(SOURCES_MOC)
	@rm -f $(OBJECTS)
	@rm -f $(MOCOBJS)
	@rm -f $(DEPFILES)
	@rm -f $(MAINDEPFILE)

depclean:
	@rm -f $(DEPFILES)
	@rm -f $(MAINDEPFILE)

distclean:
	@rm -rf build

release:
	@echo Unimplemented

doc:
	@cd doc && doxygen doxygen-conf/doxygen.conf

$(MAINDEPFILE): Makefile.sources $(DEPFILES)
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	@rm -f $@
	@echo "# DO NOT EDIT!!! (all changed will be lost!)" >> $@
	@echo "# This file has been generated automatically by Makefile from source files." >> $@
	@for file in $(DEPFILES); do \
		cat "$$file" >> $@; \
	 done;
	@for file in $(SOURCES_MOC); do \
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

$(TARGETS):
	$(call prepdir, $@)
	@echo $(_s) "LD      " $(_l) $@
	$(LD) -o $@ $^ $(LDFLAGS)

-include $(MAINDEPFILE)

