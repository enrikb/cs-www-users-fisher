BUILTINS =

objs   = mkfscript.o

all: mkfscript

mkfscript: $(objs)
	$(CXX) $(objs) -lcgicc -lm -o $@

install: cgi-bin/mkfscript

cgi-bin/mkfscript: mkfscript
	mkdir cgi-bin 2>/dev/null || true
	mkdir output 2>/dev/null || true
	mkdir helpers 2>/dev/null || true
	cp mkfscript cgi-bin/mkfscript

$(objs): %.o: %.C
	$(CXX) -O2 -W -Wall -c $< -o $@

clean:
	rm -f $(objs) mkfscript

compile_commands.json: Makefile
	compiledb --command-style --full-path -n make

.PHONY: install clean
