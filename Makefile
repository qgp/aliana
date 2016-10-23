SOURCES:=AliAnaSelector.cxx

HEADERS:=$(SOURCES:cxx=h)

CXXFLAGS:=-std=c++11 -fPIC -O2 -I$(ALICE_ROOT)/include -I$(ALICE_PHYSICS)/include

all: libAnalysis.so

test: libAnalysis.so
	time root -b -q ana.C'("filelist.txt", kFALSE, 100000)'

ana: libAnalysis.so
	time root -b -q ana.C'("filelist.txt")'

libAnalysis.so: AnalysisDict.cxx $(SOURCES)
	g++ -shared -o$@ `root-config --ldflags` $(CXXFLAGS) -I$(ROOTSYS)/include $^

AnalysisDict.cxx: $(HEADERS) LinkDef.h
	rootcint -f $@ -c $(CXXFLAGS) -p -rmf libAnalysis.rootmap -rml libAnalysis.so -rml libTreePlayer $^
