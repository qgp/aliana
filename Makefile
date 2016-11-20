SOURCES_PLOT:=src/plotter.cxx
HEADERS_PLOT:=$(SOURCES_PLOT:cxx=h)

SOURCES_ANA:=src/AliAnaSelector.cxx src/AliAnaQA.cxx src/AliTreeReaderValue.cxx
HEADERS_ANA:=$(SOURCES_ANA:cxx=h)

CXXFLAGS:=-std=c++11 -fPIC -O2 -g -I$(ALICE_ROOT)/include -I$(ALICE_PHYSICS)/include

all: libAnalysis.so libPlot.so

test: histos_local.root

ana: histos_proof.root

pdf: summary.pdf

summary.pdf: fig/*.pdf
	pdftk fig/*.pdf cat output summary.pdf

histos_local.root: src/ana.C libAnalysis.so
	time root -b -q $<'("filelist.txt", kFALSE, 10000000)'
	mv histos.root histos_local.root

histos_proof.root: src/ana.C libAnalysis.so
	time root -b -q $<'("filelist.txt")'
	mv histos.root histos_proof.root

plot_%: src/plot.C libPlot.so histos_%.root
	rm -f fig/*.pdf
	root -b -q $<'("'histos_$*.root'")'

libAnalysis.so: AnalysisDict.cxx $(SOURCES_ANA)
	g++ -shared -o$@ `root-config --ldflags` $(CXXFLAGS) -I$(ROOTSYS)/include $^

AnalysisDict.cxx: $(HEADERS_ANA) src/Analysis_LinkDef.h
	rootcint -f $@ -c $(CXXFLAGS) -p -rmf libAnalysis.rootmap -rml libAnalysis.so -rml libTreePlayer -rml libAOD $^

libPlot.so: PlotDict.cxx $(SOURCES_PLOT)
	g++ -shared -o$@ `root-config --ldflags` $(CXXFLAGS) -I$(ROOTSYS)/include $^

PlotDict.cxx: $(HEADERS_PLOT) src/Plot_LinkDef.h
	rootcint -f $@ -c $(CXXFLAGS) -p -rmf libPlot.rootmap -rml libPlot.so $^

