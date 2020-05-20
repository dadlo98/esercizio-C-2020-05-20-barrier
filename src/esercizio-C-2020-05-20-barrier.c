#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>

#define CHECK_ERR(a,msg) {if ((a) == -1) { perror((msg)); exit(EXIT_FAILURE); } }
#define N 10

sem_t barrier;
int count;
int number_of_threads = N;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * thread_function(void * arg) {
    int s;
    printf("\nrendezvous\n");

    s = pthread_mutex_lock(&mutex);
    CHECK_ERR(s,"lock");
        count++;
    s = pthread_mutex_unlock(&mutex);
    CHECK_ERR(s,"unlock");

    if(count == N) {
        s = sem_post(&barrier);
        CHECK_ERR(s,"post");
    }

    s = sem_wait(&barrier);
    CHECK_ERR(s,"wait");
    s = sem_post(&barrier);
    CHECK_ERR(s,"post");

    printf("\ncritical point\n");

    return NULL;
}

int main(int argc, char * argv[]) {
    int res;
    pthread_t threads[N];
    void * s;

    res = sem_init(&barrier,
					0, // 1 => il semaforo è condiviso tra processi,
					   // 0 => il semaforo è condiviso tra threads del processo
					0 // valore iniziale del semaforo
				  );

	CHECK_ERR(res,"sem_init");

    for(int i=0; i<number_of_threads; i++) {
        res = pthread_create(&threads[i], NULL, thread_function, NULL);
        CHECK_ERR(res,"pthread_create");
    }

    for(int i=0; i<number_of_threads; i++) {
        res = pthread_join(threads[i], &s);
        CHECK_ERR(res,"pthread_join");
    }

    res = sem_destroy(&barrier);
    CHECK_ERR(res,"sem_destroy");

    return 0;
}
