CC = gcc
EXEC1 = build/server
EXEC2 = build/client
CFLAGS= -g -Wall -std=c11 
LDLIBS=-lrt -pthread


all : $(EXEC1) $(EXEC2)
.PHONY: clean	

$(warning default goal is $(.DEFAULT_GOAL))


src/client/lpc_client.o : src/client/lpc_client.c
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)

src/server/lpc_server.o : src/server/lpc_server.c
	$(CC) $(CFLAGS)  -o $@ -c $< $(LDLIBS)
	
$(EXEC1) : src/server/lpc_server.o
	$(CC) $(CFLAGS)  -o $@ $^ $(LDLIBS)
	
$(EXEC2) : src/client/lpc_client.o
	$(CC) $(CFLAGS)  -o $@ $^ $(LDLIBS) 
	
	
# Reset the default goal.
#.DEFAULT_GOAL :=
	
clean:
	rm -rf src/client/*.o  src/server/*.o 
