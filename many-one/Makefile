final: test.o roundRobin.o thread.o encrypt.o
	cc test.o roundRobin.o thread.o encrypt.o -o final
test.o: test.c
	cc -c test.c
roundRobin.o: roundRobin.h roundRobin.c
	cc -c roundRobin.c
thread.o: thread.h thread.c
	cc -c thread.c
encrypt.o: encrypt.h encrypt.c
	cc -c encrypt.c
clean:
	rm *.o final