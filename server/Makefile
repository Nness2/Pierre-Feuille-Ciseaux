srv: server.o game.o main.o
	gcc -o srv server.o game.o main.o

server.o: server.c
	gcc -o server.o -c server.c

game.o: game.c
	gcc -o game.o -c game.c

main.o: main.c
	gcc -o main.o -c main.c

clean:
	rm *.o main