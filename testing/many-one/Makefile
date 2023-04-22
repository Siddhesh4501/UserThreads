all: final run
final: test.o roundRobin.o thread.o encrypt.o
	cc test.o roundRobin.o thread.o encrypt.o -o final
test.o: many-one-test.c
	cc -c many-one-test.c -o test.o
roundRobin.o: ../../many-one/roundRobin.h ../../many-one/roundRobin.c
	cc -c ../../many-one/roundRobin.c
thread.o: ../../many-one/thread.h ../../many-one/thread.c
	cc -c ../../many-one/thread.c
encrypt.o: ../../many-one/encrypt.h ../../many-one/encrypt.c
	cc -c ../../many-one/encrypt.c
clean:
	rm *.o final
run: final
	./final
