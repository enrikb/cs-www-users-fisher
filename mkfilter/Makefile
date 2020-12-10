BUILTINS =

CXXFLAGS = -O2 -W -Wall -std=c++98

objs = mkfscript.o

all: mkfscript

mkfscript: $(objs)
	$(CXX) $(CXXFLAGS) $(objs) -l:libcgicc.a -lm -o $@

install: mkfscript
	mkdir -p $(DESTDIR)output  2>/dev/null || true
	mkdir -p $(DESTDIR)helpers 2>/dev/null || true
	mkdir -p $(DESTDIR)www/cgi-bin 2>/dev/null || true
	mkdir -p $(DESTDIR)www/mkfilter       2>/dev/null || true
	mkdir -p $(DESTDIR)www/mkfilter/rcdoc 2>/dev/null || true
	cp mkfscript         $(DESTDIR)www/cgi-bin/mkfscript
	cp *.html *.gif      $(DESTDIR)www/mkfilter/
	cp rcdoc/rcdoc.pdf $(DESTDIR)www/mkfilter/rcdoc/

$(objs): %.o: %.C
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(objs) mkfscript

compile_commands.json: Makefile
	compiledb --command-style --full-path -n make

.PHONY: install clean
