clt: client.o main.o
	gcc -o clt client.o main.o

client.o: client.c
	gcc -o client.o -c client.c

main.o: main.c
	gcc -o main.o -c main.c

clean:
	rm *.o main
