#include <stdatomic.h>


typedef struct {
    atomic_flag flag;
} lock_t;

void lock_init(lock_t* lock);

void lock_lock(lock_t* lock);

void lock_unlock(lock_t* lock);