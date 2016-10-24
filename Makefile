SOURCES:=src/AliAnaSelector.cxx src/AliAnaQA.cxx src/plotter.cxx

HEADERS:=$(SOURCES:cxx=h)

CXXFLAGS:=-std=c++11 -fPIC -O2 -g -I$(ALICE_ROOT)/include -I$(ALICE_PHYSICS)/include

all: libAnalysis.so

test: histos_local.root

ana: histos_proof.root

pdf: summary.pdf

histos_local.root: src/ana.C libAnalysis.so
	time root -b -q $<'("filelist.txt", kFALSE, 100000)'
	mv histos.root histos_local.root

histos_proof.root: src/ana.C libAnalysis.so
	time root -b -q $<'("filelist.txt")'
	mv histos.root histos_proof.root

plot_%: src/plot.C histos_%.root
	rm -f fig/*.pdf
	root -b -q $<'("'histos_$*.root'")'

summary.pdf: fig/*.pdf
	pdftk fig/*.pdf cat output summary.pdf

libAnalysis.so: AnalysisDict.cxx $(SOURCES)
	g++ -shared -o$@ `root-config --ldflags` $(CXXFLAGS) -I$(ROOTSYS)/include $^

AnalysisDict.cxx: $(HEADERS) src/LinkDef.h
	rootcint -f $@ -c $(CXXFLAGS) -p -rmf libAnalysis.rootmap -rml libAnalysis.so -rml libTreePlayer $^
