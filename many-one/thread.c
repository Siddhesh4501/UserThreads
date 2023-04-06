#include"thread.h"
#include <signal.h>


sigset_t set;









void intiManyToOne();
int mythread_create(thread_id* tid, void* attr,void*(*funptr)(void*), void* arg);
int mythread_join(thread_id tid, void** retval);
int mythread_kill(thread_id tid,int sig);
void mythread_exit(void* ret);






void ModifyThreadSignalsMask(){
    sigfillset(&set);
    sigdelset(&set,SIGINT);
    sigdelset(&set,SIGSTOP);
    sigdelset(&set,SIGCONT);
    sigdelset(&set,SIGVTALRM);
    sigprocmask(SIG_BLOCK, &set, NULL);
}