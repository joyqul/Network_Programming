#include <iostream>
#include <pthread.h>
#define NLOOP 5

using namespace std;

int counter;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void *doit(void *vptr) {
    int val;
    for (int i = 0; i < NLOOP; ++i) {
        pthread_mutex_lock(&counter_mutex);
        val = counter;
        cout << pthread_self() << ": " << val+1 << endl;
        counter = val + 1;
        pthread_mutex_unlock(&counter_mutex);
    }
    return (NULL);
}

int main () {
    pthread_t tidA, tidB;
    
    pthread_create(&tidA, NULL, &doit, NULL);
    pthread_create(&tidB, NULL, &doit, NULL);
    pthread_join(tidA, NULL);
    pthread_join(tidB, NULL);
    return 0;
}

/* usful function
   int pthread_cond_wait(pthread_cond_t *cptr, pthread_mutex_t *mptr);
   int pthread_cond_signal(pthread_cond_t *cptr);

   pthread_mutex_t ndone_mutex = PTHREAD_MUTEX_INITIALIZER;
   pthread_cond_t ndone_cond = PTHREAD_COND_INITIALIZER;
   pthread_mutex_lock(&ndone_mutex);
   ndone++;
   pthread_cond_signal(&ndone_cond);
   pthread_mutex_unlock(&ndone_mutex);

   pthread_mutex_lock(&ndone_mutex);
   while (ndone == 0) pthread_cond_wait(&ndone_cond, &ndone_mutex);
   
   int pthread_cond_broadcast(pthread_cond_t *cptr);
   int pthread_cond_timedwait(pthread_cond_t *cptr, pthread_mutex_t *mptr, const struct timespec *abstime);
 */
