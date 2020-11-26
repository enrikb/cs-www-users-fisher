BUILTINS =

all: mkfilter mkshape mkaverage gencode genplot

mkfilter:   mkfilter.o complex.o
		    g++ -o $@ mkfilter.o complex.o -lm

mkshape:    mkshape.o complex.o
		    g++ -o $@ mkshape.o complex.o -lm

mkaverage:  mkaverage.o complex.o
		    g++ -o $@ mkaverage.o complex.o -lm

gencode:    gencode.o complex.o readdata.o
		    g++ -o $@ gencode.o complex.o readdata.o -lm

genplot:    genplot.o complex.o readdata.o
		    g++ -o $@ genplot.o complex.o readdata.o -lgd -lm

%.o:	    %.C mkfilter.h complex.h
		    g++ -o $@ -c $<

clean:
		    rm -f *.o mkfilter mkshape mkaverage gencode genplot
