AUTOMAKE_OPTIONS = foreign

# List of shared libraries to produce
lib_LTLIBRARIES = \
    libg4example01detector.la

AM_CPPFLAGS = \
    -I$(includedir) \
    -I$(OFFLINE_MAIN)/include \
    -I$(ROOTSYS)/include \
    -I$(G4_MAIN)/include \
    -I$(OPT_SPHENIX)/include

AM_LDFLAGS = \
    -L$(libdir) \
    -L$(OFFLINE_MAIN)/lib

libg4example01detector_la_LIBADD = \
  -lSubsysReco \
  -lg4detectors \
  -lg4testbench 

pkginclude_HEADERS = \
  G4Example01Subsystem.h

# I/O dictionaries have to exist for root5 and root6. For ROOT6 we need
# pcm files in addition. If someone can figure out how to make a list
# so this list of dictionaries is transformed into a list of pcm files
# following a simple naming convention, please change this accordingly and
# let me know
ROOT_IO_DICTS = 
# for root6 we need pcm and dictionaries but only for
# i/o classes. For root5 we need only dictionaries but
# those for i/o and classes available on the cmd line
# MAKEROOT6 is set in the configure.ac, true for root6
if MAKEROOT6
# this is a tweak to install files in $(libdir), automake refuses
# to install other files in libdir, this construct gets around this
pcmdir = $(libdir)
nobase_dist_pcm_DATA =
else
  ROOT5_DICTS = \
  G4Example01Subsystem_Dict.cc
endif


libg4example01detector_la_SOURCES = \
  $(ROOT5_DICTS) \
  G4Example01Detector.cc \
  G4Example01Subsystem.cc \
  G4Example01SteppingAction.cc

# Rule for generating table CINT dictionaries.
%_Dict.cc: %.h %LinkDef.h
	rootcint -f $@ @CINTDEFS@ -c $(DEFAULT_INCLUDES) $(AM_CPPFLAGS) $^

#just to get the dependency
%_Dict_rdict.pcm: %_Dict.cc ;

################################################
# linking tests

BUILT_SOURCES = testexternals.cc

noinst_PROGRAMS = \
  testexternals_g4example01


testexternals_g4example01_SOURCES = testexternals.cc
testexternals_g4example01_LDADD = libg4example01detector.la


testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

clean-local:
	rm -f *Dict* $(BUILT_SOURCES) *.pcm
