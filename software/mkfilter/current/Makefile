BUILTINS =

CXXFLAGS = -O2 -W -Wall -std=c++98 #-Ilibgd/include -Llibgd/lib

all: mkfilter mkshape mkaverage gencode genplot

mkfilter:   mkfilter.o complex.o
		    $(CXX) $(CXXFLAGS) -o $@ mkfilter.o complex.o -lm

mkshape:    mkshape.o complex.o
		    $(CXX) $(CXXFLAGS) -o $@ mkshape.o complex.o -lm

mkaverage:  mkaverage.o complex.o
		    $(CXX) $(CXXFLAGS) -o $@ mkaverage.o complex.o -lm

gencode:    gencode.o complex.o readdata.o
		    $(CXX) $(CXXFLAGS) -o $@ gencode.o complex.o readdata.o -lm

genplot:    genplot.o complex.o readdata.o
			$(CXX) $(CXXFLAGS) -o $@ genplot.o complex.o readdata.o -lgd -lm

%.o:	    %.C mkfilter.h complex.h
		    $(CXX) $(CXXFLAGS) -o $@ -c $<

install: mkfilter mkshape mkaverage gencode genplot
			mkdir -p $(DESTDIR)helpers 2>/dev/null || true
			mkdir -p $(DESTDIR)www/mkfilter/doc 2>/dev/null || true
			mkdir -p $(DESTDIR)www/software/mkfilter/current 2>/dev/null || true
			cp mkfilter mkshape mkaverage gencode genplot $(DESTDIR)helpers
			cp doc.ps.gz $(DESTDIR)www/mkfilter/doc/
			cp *.C *.h $(DESTDIR)www/software/mkfilter/current
clean:
		    rm -f *.o mkfilter mkshape mkaverage gencode genplot

.PHONY: install clean
