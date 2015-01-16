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
