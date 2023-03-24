/* A code demonstrating the race problem
*/
#include <stdio.h>
#include <pthread.h>



#include <stdatomic.h>

typedef struct {
    atomic_flag flag;
} spinlock_t;

void spinlock_init(spinlock_t* lock) {
    atomic_flag_clear(&lock->flag);
}

void spinlock_lock(spinlock_t* lock) {
    while (atomic_flag_test_and_set(&lock->flag))
        ; // spin
}

void spinlock_unlock(spinlock_t* lock) {
    atomic_flag_clear(&lock->flag);
}


spinlock_t l;








long c = 0, c1 = 0, c2 = 0, run = 1;
void *thread1(void *arg) {
    while(1){
        spinlock_lock(&l);
        c++;
        spinlock_unlock(&l);
        c1++;
    }
}
void *thread2(void *arg) {

    while(1){
        spinlock_lock(&l);
        c++;
        spinlock_unlock(&l);
        c2++;
    }
}
int main() {
	pthread_t th1, th2; 
    spinlock_init(&l);
	pthread_create(&th1, NULL, thread1, NULL);
	pthread_create(&th2, NULL, thread2, NULL);
	//fprintf(stdout, "Ending main\n");
	sleep(2);
	fprintf(stdout, "c = %ld c1+c2 = %ld c1 = %ld c2 = %ld \n", c, c1+c2, c1, c2);
	fflush(stdout);
}