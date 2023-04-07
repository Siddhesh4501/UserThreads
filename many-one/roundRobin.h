#include "thread.h"


void addToQueue(singlyLL* sll, thread* currthread);
thread* getRunnableThread(singlyLL* sll);
thread* moveThreadToEnd(singlyLL* sll, thread* currthread);
thread* removeThread(singlyLL* sll, thread* currthread);



