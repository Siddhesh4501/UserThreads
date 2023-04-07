#include "thread.h"


void addToSLL(singlyLL* sll, thread* currthread);
thread* getThread(singlyLL* sll, thread_id tid);
thread* getRunnableThread(singlyLL* sll);
void moveThreadToEnd(singlyLL* sll, thread* currthread, thread* prev);
thread* removeThread(singlyLL* sll, thread* currthread);



