final: test.o locks.o mythread.o singlyLL.o
	cc test.o locks.o mythread.o singlyLL.o -o final
test.o: test.c
	cc -c test.c
locks.o: locks.h locks.c
	cc -c locks.c
mythread.o: mythread.h mythread.c
	cc -c mythread.c
singlyLL.o: singlyLL.h singlyLL.c
	cc -c singlyLL.c
clean:
	rm *.o final