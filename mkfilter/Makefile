BUILTINS =

objs   = mkfscript.o

all: mkfscript

mkfscript: $(objs)
	g++ $(objs) -lcgicc -lm -o $@

install: cgi-bin/mkfscript

cgi-bin/mkfscript: mkfscript
	mkdir cgi-bin 2>/dev/null || true
	cp mkfscript cgi-bin/mkfscript

$(objs): %.o: %.C
	g++ -O2 -W -Wall -c $< -o $@

clean:
	rm -f $(objs) mkfscript

.PHONY: install clean
