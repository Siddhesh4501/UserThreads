#include "locks.h"

void lock_init(lock_t* lock) {
    atomic_flag_clear(&lock->flag);
}

void lock_lock(lock_t* lock) {
    while (atomic_flag_test_and_set(&lock->flag));
}


void lock_unlock(lock_t* lock) {
    atomic_flag_clear(&lock->flag);
}