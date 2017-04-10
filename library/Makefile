## -*- Makefile -*-
##
## User: drordas
## Time: 02-oct-2015 16:59:33
##
##

CC=gcc -g -O3 -fPIC -pthread -D_REENTRANT -D_THREAD_SAFE -D_GNU_SOURCE 
CFLAGS=-c -Wall 
OPTS= -I/usr/lib -I/usr/include -L/usr/lib -I/library -L/library -L/usr/include
LIBS=-lpthread -ldl -lc -lpcre -lm

all: libutils.a

linked_list.o :
	$(CC) $(CFLAGS) $(OPTS) linked_list.c -o linked_list.o $(LIBS)
	
pcre_regex_utils.o : 
	$(CC) $(CFLAGS) $(OPTS) pcre_regex_util.c -o pcre_regex_utils.o $(LIBS)
	
string_utils.o : 
	$(CC) $(CFLAGS) $(OPTS) string_util.c -o string_utils.o $(LIBS)
	
linked_vector.o : generic_vector.o string_vector.o
	$(CC) $(CFLAGS) $(OPTS) linked_vector.c -o linked_vector.o $(LIBS)
	
fileutils.o : filedata.o
	$(CC) $(CFLAGS) $(OPTS) fileutils.c -o fileutils.o $(LIBS)
	
string_vector.o : 
	$(CC) $(CFLAGS) $(OPTS) string_vector.c -o string_vector.o $(LIBS)

hashmap.o :	
	$(CC) $(CFLAGS) $(OPTS) hashmap.c -o hashmap.o $(LIBS)
	
generic_vector.o:
	$(CC) $(CFLAGS) $(OPTS) generic_vector.c -o generic_vector.o $(LIBS)
	
linkedhashmap.o	:
	$(CC) $(CFLAGS) $(OPTS) linkedhashmap.c -o linkedhashmap.o $(LIBS)
	
genetic.o :
	$(CC) $(CFLAGS) $(OPTS) genetic.c -o genetic.o $(LIBS)
	
mating_pool.o :
	$(CC) $(CFLAGS) $(OPTS) mating_pool.c -o mating_pool.o $(LIBS)
	
filedata.o :
	$(CC) $(CFLAGS) $(OPTS) filedata.c -o filedata.o $(LIBS)
	
sorted_cache.o :
	$(CC) $(CFLAGS) $(OPTS) sorted_cache.c -o sorted_cache.o $(LIBS)
	
logger.o : 
	$(CC) $(CFLAGS) $(OPTS) logger.c -o logger.o $(LIBS)
	
list_files.o :
	$(CC) $(CFLAGS) $(OPTS) list_files.c -o list_files.o $(LIBS)
	
dlinked_list.o :
	$(CC) $(CFLAGS) $(OPTS) dlinked_list.c -o dlinked_list.o $(LIBS)
	
dlinkedhashmap.o :
	$(CC) $(CFLAGS) $(OPTS) dlinkedhashmap.c -o dlinkedhashmap.o $(LIBS)

dictionary.o :
	$(CC) $(CFLAGS) $(OPTS) dictionary.c -o dictionary.o $(LIBS)	

experimental.o :
	$(CC) $(CFLAGS) $(OPTS) experimental.c -o experimental.o $(LIBS)	
	
iniparser.o :
	$(CC) $(CFLAGS) $(OPTS) iniparser.c -o iniparser.o $(LIBS)	
	
libutils.a : pcre_regex_utils.o string_utils.o linked_list.o generic_vector.o linked_vector.o fileutils.o string_vector.o hashmap.o linkedhashmap.o genetic.o mating_pool.o filedata.o sorted_cache.o logger.o list_files.o dictionary.o iniparser.o experimental.o dlinked_list.o dlinkedhashmap.o
	ar rcs libutils.a pcre_regex_utils.o string_utils.o linked_list.o generic_vector.o linked_vector.o fileutils.o string_vector.o hashmap.o linkedhashmap.o genetic.o mating_pool.o filedata.o sorted_cache.o logger.o list_files.o dictionary.o iniparser.o experimental.o dlinked_list.o dlinkedhashmap.o
	
clean:
	rm *.o *.a
