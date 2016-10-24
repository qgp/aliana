SOURCES:=AliAnaSelector.cxx AliAnaQA.cxx

HEADERS:=$(SOURCES:cxx=h)

CXXFLAGS:=-std=c++11 -fPIC -O2 -g -I$(ALICE_ROOT)/include -I$(ALICE_PHYSICS)/include

all: libAnalysis.so

test: libAnalysis.so
	time root -b -q ana.C'("filelist.txt", kFALSE, 100000)'

ana: histos.root

histos.root: libAnalysis.so
	time root -b -q ana.C'("filelist.txt")'

plot: plot.C histos.root
	rm fig/*.pdf
	root -b -q $<

pdf: summary.pdf

summary.pdf: fig/*.pdf
	pdftk fig/*.pdf cat output summary.pdf

libAnalysis.so: AnalysisDict.cxx $(SOURCES)
	g++ -shared -o$@ `root-config --ldflags` $(CXXFLAGS) -I$(ROOTSYS)/include $^

AnalysisDict.cxx: $(HEADERS) LinkDef.h
	rootcint -f $@ -c $(CXXFLAGS) -p -rmf libAnalysis.rootmap -rml libAnalysis.so -rml libTreePlayer $^
