CFLAGS = -Wall -std=c++11 

main: clean
	g++ server.cc methods.cc socketMethods.cc -o server $(CFLAGS)
	g++ client.cc methods.cc socketMethods.cc -o client $(CFLAGS)


server.o: server.cc
	g++ -c server.cc methods.cc $(CFLAGS)

client.o: client.cc
	g++ -c client.cc methods.cc $(CFLAGS)



clean:
	rm -f *.o server client
