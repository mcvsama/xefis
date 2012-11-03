# vim:ts=4

LIBS			=
PKGCONFIG_LIBS	=
PKGCONFIG_FLAGS	=
CXX				= g++
DEPCC			= $(CXX)
CXXFLAGS		= -c -std=c++11 -Wall -Wall -Wextra -Wunused  -Wunused-function -Wunused-label -fstrict-aliasing -Wstrict-aliasing=3 -Wnoexcept -fnothrow-opt $(PKGCONFIG_FLAGS)
SO				= g++ -shared
SOFLAGS			=
LD				= g++
LDFLAGS			= $(foreach lib, $(LIBS), -l$(lib)) $(PKGCONFIG_LIBS)
AR				= ar
MOC				= $(QT_PREFIX)/bin/moc

.PHONY: first all dep help dist clean depclean distclean release releases
.DEFAULT: all
MAKEFLAGS = --silent

HEADERS =
SOURCES =
OBJECTS =
NODEP_OBJECTS =
MOCSRCS =
TARGETS =

#### Helpers ####

distdir		= .build/$(DIST)
reldir		= release/$(DIST)
reldir_pkg	= $(reldir)/xefix-$(XEFIS_VERSION)-$(DIST)
prepdir		= $(shell mkdir -p "$$(dirname "$(strip $(1))")")
mkmocs		= $(foreach file, $(1), $(dir $(file))$(shell basename '$(file)' $(suffix $(file))).moc.cc)
mkobjs		= $(foreach file, $(1), $(distdir)/$(dir $(file))$(shell basename '$(file)' $(suffix $(file))).o)
mkdeps		= $(foreach file, $(1), $(distdir)/$(dir $(file))$(shell basename '$(file)' $(suffix $(file))).dep)

ifeq ($(shell tput colors), -1)
	_s = ""
	_l = ""
else
	_s = "[1;34m"
	_l = "[0m"
endif

#### Makefiles ####

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
	@echo "namespace Xefis { namespace Version {" >>$@
	@echo "	const char* commit = \"$(COMMIT_REAL)\";" >>$@
	@echo "	const char* branch = \"$(BRANCH_REAL)\";" >>$@
	@echo "	const char* version = \"$(XEFIS_VERSION)\";" >>$@
	@echo "} }" >>$@

$(call mkobjs, $(VERSION_FILE)): $(VERSION_FILE)
endif # GIT_AVAILABLE == OK
endif # VERSION_FILE != ""

#### Rules ####

DEPFILES := $(call mkdeps, $(subst $(VERSION_FILE),,$(SOURCES)))

all: $(distdir)/Makefile.dep $(DEPFILES) $(TARGETS)

dep: $(DEPFILES)

help:
	@echo 'Available targets:'
	@echo '  all        Compiles program.'
	@echo '  dep        Generates dependencies.'
	@echo '  clean      Cleans source tree and dep files'
	@echo '  distclean  Cleans dist directory.'
	@echo '  release    Creates release.'
	@echo '  releases   Creates all configured releases.'
	@echo '  help       Shows this help.'

clean:
	@rm -f $(MOCSRCS)
	@rm -f $(DEPFILES)

distclean:
	@rm -f $(DEPFILES) $(MOCOBJS) $(OBJECTS) $(NODEP_OBJECTS)

release:
	@echo Making release: $(DIST)
	@make clean distclean all
	@rm -rf $(reldir)
	@mkdir -p $(reldir_pkg)
	@mkdir -p $(reldir_pkg)/share
	@cp -Ra ../share/config $(reldir_pkg)/share/
	@mkdir -p $(reldir_pkg)/share/images/16
	@mkdir -p $(reldir_pkg)/share/images/22
	@cp -Ra ../share/images/16/*.png $(reldir_pkg)/share/images/16/
	@cp -Ra ../share/images/22/*.png $(reldir_pkg)/share/images/22/
	@cp -Ra ../share/images/*.png $(reldir_pkg)/share/images/
	@cp $(distdir)/xefis.run $(reldir_pkg)/xefis
ifneq ($(DIST),debug)
	@strip $(reldir_pkg)/xefis
endif
	@cd $(reldir) && tar zcf xefis-$(XEFIS_VERSION)-$(DIST).tar.gz xefis-$(XEFIS_VERSION)-$(DIST) && cd -

releases:
	@for r in $(DISTS); do DIST="$$r" make release; done

$(distdir)/Makefile.dep: Makefile Makefile.sources $(DEPFILES)
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	@rm -f $@
	@echo "# DO NOT EDIT!!! (all changed will be lost!)" >> $@
	@echo "# This file has been generated automatically by Makefile from source files." >> $@
	@for file in $(DEPFILES); do \
		cat "$$file" >> $@; \
	 done;
	@for file in $(MOCSRCS); do \
		echo "$(distdir)/$$(dirname "$$file")/$$(basename "$$file" .cc).o: $$file" >> $@; \
	 done;

$(distdir)/%.dep: %.cc %.h
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	$(DEPCC) $(CXXFLAGS) -MM -MQ "$(distdir)/$$(dirname "$<")/$$(basename "$<" $$(expr "$<" : '.*\(\..*\)$$')).o" $< > $@

$(distdir)/%.dep: %.cc
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	$(DEPCC) $(CXXFLAGS) -MM -MQ "$(distdir)/$$(dirname "$<")/$$(basename "$<" $$(expr "$<" : '.*\(\..*\)$$')).o" $< > $@

$(distdir)/%.dep: %.h
	$(call prepdir, $@)
	@echo $(_s) "DEP     " $(_l) $@
	$(DEPCC) $(CXXFLAGS) -MM -MQ "$(distdir)/$$(dirname "$<")/$$(basename "$<" $$(expr "$<" : '.*\(\..*\)$$')).o" $< > $@

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

%.moc.cc: %.h
	@echo $(_s) "MOC     " $(_l) $@
	$(MOC) $< -o $@

$(LINKEDS):
	$(call prepdir, $@)
	@echo $(_s) "LD      " $(_l) $@
	$(LD) -o $@ $(NODEP_OBJECTS) $(OBJECTS) $(MOCOBJS) $(LDFLAGS)

-include $(distdir)/Makefile.dep

