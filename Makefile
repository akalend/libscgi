all:
	g++  example.cpp -o server -Wall -g  -L.  -L/usr/local/lib -levent -lscgi

lib:
	gcc -c  scgiServer.cpp parser.cpp -I/usr/local/include
	ar rcs libscgi.a *o
