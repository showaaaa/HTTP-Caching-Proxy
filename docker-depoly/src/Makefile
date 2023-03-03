# all: proxy

# proxy: proxy.cpp socket.h 
# 	g++ -g -o proxy proxy.cpp -lpthread

# .PHONY:
# 	clean
# clean:
# 	rm -rf *.o main
CC = g++
CFLAGS = -g -Wall
LDFLAGS = -lpthread
objects = proxy.o sbuf.o request.o function.o response.o
all: proxy

function.o: 
	$(CC) $(CFLAGS) -c function.cpp

sbuf.o:
	$(CC) $(CFLAGS) -c sbuf.cpp

request.o:
	$(CC) $(CFLAGS) -c request.cpp

response.o:
	$(CC) $(CFLAGS) -c response.cpp

proxy.o: proxy.cpp
	$(CC) $(CFLAGS) -c proxy.cpp

proxy: $(objects)
	g++ $(CFLAGS) $(objects) -o proxy $(LDFLAGS)


clean:
	rm -f *~ *.o proxy *.gch
