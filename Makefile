hls.o: hls.c hls.h
	cc -c -Wall hls.c

.PHONY: clean
clean:
	rm -f hls.o
