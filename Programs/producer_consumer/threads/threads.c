#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<errno.h>
#include<string.h>

#define MAX 8

int array[MAX];

struct shared {
	pthread_mutex_t mutex;
	int nput;
	int val;	
}shared_variable = {
 PTHREAD_MUTEX_INITIALIZER
};

struct condition {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int ready;
}condition_variable = {
PTHREAD_MUTEX_INITIALIZER,
PTHREAD_COND_INITIALIZER
};

void *
producer(void *arg) {
	printf("\n\n Producer No : %d", (*(int*)arg));
	while(1) {
		printf("\n\n Producer No : %d  => Aquiring Mutex", (*(int*)arg));
		pthread_mutex_lock(&shared_variable.mutex);
		if(shared_variable.nput > MAX) {
			pthread_mutex_unlock(&shared_variable.mutex);
			printf("\n\n Producer No : %d  => Unlocking Mutex", (*(int*)arg));
			return;
		}
		array[shared_variable.nput] = shared_variable.val;
		shared_variable.nput++;
		shared_variable.val++;
		pthread_mutex_unlock(&shared_variable.mutex);
		printf("\n\n Producer No : %d  => Unlocking Mutex", (*(int*)arg));

		printf("\n\n Producer No : %d  => locking Mutex for condition variable", (*(int*)arg));
		pthread_mutex_lock(&condition_variable.mutex);
		if(condition_variable.ready == 0) {
			pthread_cond_signal(&condition_variable.cond);
			printf("\n\n Producer No : %d  => signaling  condition variable", (*(int*)arg));
		}
		condition_variable.ready++;
		pthread_mutex_unlock(&condition_variable.mutex);
		printf("\n\n Producer No : %d  => unlocking Mutex for condition variable", (*(int*)arg));
	}
	return NULL;
}

void *
consumer(void *arg) {
	int i;
	printf("\n\n In Consumer");
	for(i = 0 ; i < MAX ; i++) {
		printf("\n\n Consumer waiting for locking condition variable");
		pthread_mutex_lock(&condition_variable.mutex);
		while(condition_variable.ready == 0) {
			pthread_cond_wait(&condition_variable.cond, &condition_variable.mutex);				
		}
		condition_variable.ready--;
		printf("\n\nValue of Array[%d] = %d",i,array[i]);
		pthread_mutex_unlock(&condition_variable.mutex);
		printf("\n\n Consumer unlocking condition variable");
	}
	return NULL;
}

void
threads_handling(int argc, char** argv) {
	int nthreads, i;
	pthread_t *producer_threads, consumer_thread;
	
	nthreads = atoi(argv[1]);
	producer_threads = (pthread_t *)calloc(sizeof(pthread_t), nthreads);

	pthread_setconcurrency(nthreads);
	for(i = 0; i<nthreads; i++) {
		if(pthread_create(&producer_threads[i], NULL, &producer, &i) != 0) {
			fprintf(stderr, "Error in creating producer no %d  as %s", i+1, strerror(errno));
			return;
		}
	}
	for(i = 0 ; i<nthreads; i++) {
		if(pthread_join(producer_threads[i], NULL) != 0) {
			fprintf(stderr, "Error in joining producer no %d as %s", i+1, strerror(errno));
			return;
		}
	}
	if(pthread_create(&consumer_thread, NULL, &consumer, NULL) != 0) {
		fprintf(stderr, "Error in creating producer no %d  as %s", i+1, strerror(errno));
		return;
	}
	if(pthread_join(consumer_thread, NULL) != 0) {
		fprintf(stderr, "Error in joining producer no %d as %s", i+1, strerror(errno));
		return;
	}
}


int
main(int argc, char **argv) {
	if(argc != 2) {
		fprintf(stderr, "Invalid no. of arguments. Two arguments required.");
		return -1;
	}
	threads_handling(argc, argv);
	return 0;
}	
