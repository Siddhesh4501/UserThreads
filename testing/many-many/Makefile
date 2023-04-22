all: final run
final: test.o roundRobin.o thread.o encrypt.o locks.o 
	cc test.o roundRobin.o thread.o encrypt.o locks.o -o final
test.o: many-many-test.c
	cc -c many-many-test.c -o test.o
roundRobin.o: ../../many-many/roundRobin.h ../../many-many/roundRobin.c
	cc -c ../../many-many/roundRobin.c
locks.o: ../../many-many/locks.h ../../many-many/locks.c
	cc -c ../../many-many/locks.c
thread.o: ../../many-many/thread.h ../../many-many/thread.c
	cc -c ../../many-many/thread.c
encrypt.o: ../../many-many/encrypt.h ../../many-many/encrypt.c
	cc -c ../../many-many/encrypt.c
clean:
	rm *.o final
run: final
	./final
