OBJS= main.o

UnixLs: $(OBJS)
	gcc -o UnixLs $(OBJS)

main.o: main.c
	gcc -c main.c


clean:
	rm UnixLs $(OBJS)
