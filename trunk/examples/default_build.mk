# This is the default Makefile for MBSim examples.
# It builds a "main" executable from all sources found under $(SRCDIR).
# SRCDIR must be set externally.
# PACKAGES must also be set externally to a list of all pkg-config names required by the example.
# Moreover the LDFLAGS, CPPFLAGS and CXXFLAGS are honored if set externally.

# Enable VPATH builds with sources at SRCDIR
VPATH=$(SRCDIR)

# use a sources all *.cc file under SRCDIR
SOURCES:=$(shell (cd $(SRCDIR); find -name "*.cc"))
# object and dependency files (derived from SOURCES)
OBJECTS=$(SOURCES:.cc=.o)
DEPFILES=$(SOURCES:.cc=.d)

# default target
all: main

# link main executable with pkg-config options from PACKAGES (runexamples.py executes always ./main)
main: $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(shell pkg-config --libs $(PACKAGES))

rpath: $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(shell pkg-config --libs $(PACKAGES))  $(shell pkg-config --libs-only-L $(PACKAGES) | sed 's/-L/-Wl,-rpath,/g')

# compile source with pkg-config options from PACKAGES
%.o: %.cc
	$(CXX) -c -o $@ $< $(CPPFLAGS) $(CXXFLAGS) $(shell pkg-config --cflags $(PACKAGES))

# generate make rules for all source files using gcc -M with pkg-config options from PACKAGES
%.d: %.cc
	$(CXX) -M $(CPPFLAGS) $(shell pkg-config --cflags $(PACKAGES)) $< > $@

# clean target: remove all generated files
clean:
	rm -f main $(OBJECTS) $(DEPFILES)

# include the generated make rules (without print a warning about missing include files (at first run))
-include $(DEPFILES)
