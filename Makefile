CFLAGS=-lpthread -lncurses
FILES=locksync elevator.log hw5

locksync: locksync.c main.c elevator.h
	gcc $^ -o $@ $(CFLAGS)

hw5: hw5.c main.c elevator.h
	gcc $^ -o $@ $(CFLAGS)

clean:
	rm -f $(FILES) *.o
