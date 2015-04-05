
#include "shared.h"
#include<stdlib.h>

static struct shared_mem *sharedptr;

static int
write_in_shared_mem() {
	char string[32];
	int i, value;
	while(1) {
		for(i=0 ; i<MAX ; i++) {
			printf("\n\n wait for nempty");
			sem_wait(&sharedptr->nempty);
			if(sem_getvalue(&sharedptr->nempty, &value) == -1) {
				fprintf(stderr, "Error in sem_getvalue  nempty : ", strerror(errno));
				return -1;
			}
			printf("\n Value for nempty : %d", value);	
			
			printf("\n\n wait for mutex");
			sem_wait(&sharedptr->mutex);
			if(sem_getvalue(&sharedptr->mutex, &value) == -1) {
				fprintf(stderr, "Error in sem_getvalue  mutex : ", strerror(errno));
				return -1;
			}
			printf("\n Value for mutex : %d", value);	
			
			sharedptr->data[i].id = i+1;
			printf("Enter the name for ID = %d    :  ", (i+1));
			scanf("%s", string);
			strcpy(sharedptr->data[i].name, string);
			printf(" Copied string  :: %s", sharedptr->data[i].name);
	
			printf("\n\n post for mutex");
			sem_post(&sharedptr->mutex);
			if(sem_getvalue(&sharedptr->mutex, &value) == -1) {
				fprintf(stderr, "Error in sem_getvalue  mutex : ", strerror(errno));
				return -1;
			}
			printf("\n Value for mutex : %d", value);	
			
			printf("\n\n Post for nstored");
			sem_post(&sharedptr->nstored);
			if(sem_getvalue(&sharedptr->nstored, &value) == -1) {
				fprintf(stderr, "Error in sem_getvalue  mutex : ", strerror(errno));
				return -1;
			}
			printf("\n Value for nstored : %d", value);	
		}
	}
	return 0;	
}

static int
init_sems() {
        printf("Creating sem 1\n");
        if(sem_init(&sharedptr->nempty, 1, MAX) == -1) {
                fprintf(stderr, "sem_init() error %s", strerror(errno));
                return -1;
        }
        printf("Creating sem 2\n");
        if(sem_init(&sharedptr->nstored, 1, 0) == -1) {
                fprintf(stderr, "sem_init() error %s", strerror(errno));
                return -1;
        }
        printf("Creating sem 3\n");
        if(sem_init(&sharedptr->mutex, 1, 1) == -1) {
                fprintf(stderr, "sem_init() error %s", strerror(errno));
                return -1;
        }
        int value;
        if(sem_getvalue(&sharedptr->nempty ,&value) == -1) {
                fprintf(stderr, "Error in sem_getvalue = %s", strerror(errno));
                return -1;
        }
        printf("Value of Semaphore nempty = %d\n", value);

        if(sem_getvalue(&sharedptr->nstored ,&value) == -1) {
                fprintf(stderr, "Error in sem_getvalue = %s", strerror(errno));
                return -1;
        }
        printf("Value of Semaphore nstored = %d\n", value);

        if(sem_getvalue(&sharedptr->mutex ,&value) == -1) {
                fprintf(stderr, "Error in sem_getvalue = %s", strerror(errno));
                return -1;
        }
        printf("Value of Semaphore mutex = %d\n", value);
        return 0;
}

static int
create_shared_mem() {
	int fd;
	int exists = 0;
	int size;
	struct stat buf;
 
	if((fd = shm_open(MEM_NAME, O_CREAT | O_RDWR, 0644)) == -1) {
		fprintf(stderr, "Error in creating shared memory in writer => %s", strerror(errno));
		return -1;
	}
	
	if(exists) {
		fstat(fd, &buf);
		size = buf.st_size;
	}
	else {
		ftruncate(fd, sizeof(struct shared_mem));
		size = sizeof(struct shared_mem);
	}

	if((sharedptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		fprintf(stderr, "Error in mmap in writer => %s", strerror(errno));
		return -1;
	}
	close(fd);
	
	if(!exists) {
		if(init_sems() == -1) {
			return -1;
		}
	}
	return 0;
}

static int
destroy_sems() {
        printf("Destroying sem 1\n");
        if(sem_destroy(&sharedptr->mutex) == -1) {
                fprintf(stderr, "sem_destroy() error %s", strerror(errno));
                return -1;
        }
        printf("Destroying sem 2\n");
        if(sem_destroy(&sharedptr->nstored) == -1) {
                fprintf(stderr, "sem_destroy() error %s", strerror(errno));
                return -1;
        }
        printf("Destroying sem 3\n");
        if(sem_destroy(&sharedptr->nempty) == -1) {
                fprintf(stderr, "sem_destroy() error %s", strerror(errno));
                return -1;
        }
        return 0;
}


void
delete_shared_mem() {
	if(destroy_sems() == -1)
                return;
        if(munmap(sharedptr, sizeof(struct shared_mem)) == -1) {
                fprintf(stderr, "Error in munmap in writer => %s", strerror(errno));
                return;
        }
        if(shm_unlink(MEM_NAME) == -1) {
                fprintf(stderr, "Error in shm_unlink in writer => %s", strerror(errno));
                return;
        }
}

int
main(void) {
	atexit(&delete_shared_mem);	
	if(create_shared_mem() == -1) 
		goto exit;
	if(write_in_shared_mem() == -1)
		goto exit;

exit:
	return 0;
}
