all: final run
final: test.o locks.o mythread.o singlyLL.o
	cc test.o locks.o mythread.o singlyLL.o -o final
test.o: one-one-test.c
	cc -c one-one-test.c -o test.o
locks.o: ../../one-one/locks.h ../../one-one/locks.c
	cc -c ../../one-one/locks.c
mythread.o: ../../one-one/mythread.h ../../one-one/mythread.c
	cc -c ../../one-one/mythread.c
singlyLL.o: ../../one-one/singlyLL.h ../../one-one/singlyLL.c
	cc -c ../../one-one/singlyLL.c
clean:
	rm *.o final
run: final
	./final
