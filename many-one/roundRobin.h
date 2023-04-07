#include "thread.h"


void addToQueue(singlyLL* sll, thread* currthread);
thread* getRunnableThread(singlyLL* sll);
void moveThreadToEnd(singlyLL* sll, thread* currthread, thread* prev);
thread* removeThread(singlyLL* sll, thread* currthread);



