CC=gcc -g -O3 -fPIC -pthread -D_REENTRANT -D_THREAD_SAFE -D_GNU_SOURCE 
OPTS=-I/usr/lib -I/usr/include -L/usr/lib -Ilibrary -Llibrary -L/usr/include
LIBS=-lpthread -ldl -lc -lutils -lpcre -lm

#all : libutils.a autoregex xvalidation run_regex
all : libutils.a discoverRegex
	
libutils.a: 
	make -C library

discoverRegex : libutils.a
	$(CC) $(OPTS) $(LIBS) run_regex.c -o discoverRegex $(LIBS)	
	
clean :
	make clean -C library
	rm discoverRegex