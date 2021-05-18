CC = gcc
EXEC1 = build/server
EXEC2 = build/client
CFLAGS= -g -Wall -std=c11 
LDLIBS=-lrt -pthread -L/build


all : $(EXEC1) $(EXEC2)
.PHONY: clean	

$(warning default goal is $(.DEFAULT_GOAL))

build/modify_all.o : include/fonctions_client/modify_all.c  include/fonctions_client/modify_all.h
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)

build/modify_wrong_lpc_string.o : include/fonctions_client/modify_wrong_lpc_string.c  include/fonctions_client/modify_wrong_lpc_string.h
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)

build/modify_lpc_string.o : include/fonctions_client/modify_lpc_string.c  include/fonctions_client/modify_lpc_string.h
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)

build/add_int.o : include/fonctions_client/add_int.c  include/fonctions_client/add_int.h
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)

build/print_lpc_string.o : include/fonctions_client/print_lpc_string.c  include/fonctions_client/print_lpc_string.h
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)

build/libfonctionsclient.a : build/add_int.o build/print_lpc_string.o build/modify_lpc_string.o build/modify_wrong_lpc_string.o build/modify_all.o
	ar rcu build/libfonctionsclient.a build/add_int.o build/print_lpc_string.o build/modify_lpc_string.o build/modify_wrong_lpc_string.o build/modify_all.o
	ranlib build/libfonctionsclient.a


build/lpc_functions.o : include/lpc_functions/lpc_functions.c  include/lpc_functions/lpc_functions.h
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)

build/init_memory.o : include/Memoire/init_memory.c  include/Memoire/init_memory.h
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)

build/init_mutex.o: include/MutexCond/init_mutex.c  include/MutexCond/init_mutex.h
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)
	

build/lpc_client.o : src/client/lpc_client.c
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)

build/lpc_server.o : src/server/lpc_server.c
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)
	
$(EXEC1) : build/lpc_server.o  build/init_memory.o  build/init_mutex.o  build/lpc_functions.o build/libfonctionsclient.a
	$(CC) $(CFLAGS)  -o $@ $^ $(LDLIBS) 
	
$(EXEC2) : build/lpc_client.o  build/lpc_functions.o
	$(CC) $(CFLAGS)  -o $@ $^ $(LDLIBS) 
	
libs: build/libfonctionsclient.a	
# Reset the default goal.
#.DEFAULT_GOAL :=
	
clean:
	rm -rf build/*
