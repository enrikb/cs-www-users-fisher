BUILTINS =

cgibin = /york/www/usr/fisher/cgi-bin
objs   = mkfscript.o

mkfscript: $(objs)
	g++ $(objs) -lcgicc -lm -o $@

install:
	cp mkfscript $(cgibin)/mkfscript

$(objs): %.o: %.C
	g++ -W -Wall -c $< -o $@

clean:
	rm -f $(objs) mkfscript

.PHONY: install clean
