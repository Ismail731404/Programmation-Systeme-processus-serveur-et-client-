CC = gcc
EXEC1 = build/server
EXEC2 = build/client
CFLAGS= -g -Wall -std=c11 
LDLIBS=-lrt -pthread


all : $(EXEC1) $(EXEC2)
.PHONY: clean	

$(warning default goal is $(.DEFAULT_GOAL))

build/init_memory.o : include/Memoire/init_memory.c  include/Memoire/init_memory.h
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)

build/init_mutex.o: include/MutexCond/init_mutex.c  include/MutexCond/init_mutex.h
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)

build/lpc_client.o : src/client/lpc_client.c
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)

build/lpc_server.o : src/server/lpc_server.c
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)
	
$(EXEC1) : build/lpc_server.o build/init_memory.o  build/init_mutex.o
	$(CC) $(CFLAGS)  -o $@ $^ $(LDLIBS)
	
$(EXEC2) : build/lpc_client.o
	$(CC) $(CFLAGS)  -o $@ $^ $(LDLIBS) 
	
	
# Reset the default goal.
#.DEFAULT_GOAL :=
	
clean:
	rm -rf build/*.o  build/*.o 
