BUILTINS =

cgibin = /york/www/usr/fisher/cgi-bin
objs   = lcfilter.o coeffs.o

lcfilter:   $(objs)
		    $(CXX) $(objs) -lm -lcgicc -o $@

%.o:	    %.C lcfilter.h
		    $(CXX) -O2 -c $< -o $@

clean:
		    rm -f *.o lcfilter

compile_commands.json: Makefile
	compiledb --command-style --full-path -n make

.PHONY: clean
