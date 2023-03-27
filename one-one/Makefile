final: test3.o locks.o mythread.o singlyLL.o
	cc test3.o locks.o mythread.o singlyLL.o -o final
test3.o: test3.c
	cc -c test3.c
locks.o: locks.h locks.c
	cc -c locks.c
mythread.o: mythread.h mythread.c
	cc -c mythread.c
singlyLL.o: singlyLL.h singlyLL.c
	cc -c singlyLL.c
clean:
	rm *.o final