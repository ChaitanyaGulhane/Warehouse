
#include "shared.h"
#include<stdlib.h>

static struct shared_mem *sharedptr;

static int
read_from_shared_mem() {
	int i, value;
	while(1) {
		for(i=0 ; i<MAX ; i++) {
			printf("Wait for nstored\n");
			sem_wait(&sharedptr->nstored);
			if(sem_getvalue(&sharedptr->nstored, &value) == -1) {
				fprintf(stderr, "Error in sem_getvalue nstored : %s", strerror(errno));
				return -1;
			}
			printf("Value of nstored = %d\n\n", value);
		
			printf("Wait for mutex\n");
			sem_wait(&sharedptr->mutex);
			if(sem_getvalue(&sharedptr->mutex, &value) == -1) {
				fprintf(stderr, "Error in sem_getvalue mutex : %s", strerror(errno));
				return -1;
			}
			printf("Value of mutex = %d\n\n", value);
			
			printf("The ID-name pair = %d %s  \n\n", sharedptr->data[i].id, sharedptr->data[i].name);
			
			printf("Post for mutex\n");
			sem_post(&sharedptr->mutex);
			if(sem_getvalue(&sharedptr->mutex, &value) == -1) {
				fprintf(stderr, "Error in sem_getvalue mutex : %s", strerror(errno));
				return -1;
			}
			printf("Value of mutex = %d\n\n", value);
			
			printf("Post for nempty\n");
			sem_post(&sharedptr->nempty);
			if(sem_getvalue(&sharedptr->nempty, &value) == -1) {
				fprintf(stderr, "Error in sem_getvalue nempty : %s", strerror(errno));
				return -1;
			}
			printf("Value of nempty = %d\n\n", value);
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
	int protection;

	printf("\n Accessing Shared mem");
	if((fd = shm_open(MEM_NAME, O_CREAT | O_EXCL | O_RDWR, 0644)) == -1) {
		fprintf(stderr, "\nError in creating shared memory in reader => %s\n", strerror(errno));
		if(errno == EEXIST)
			exists = 1;
		else
			return -1;
	}
	
	if(exists) {
		printf("\nExists 1 \n");
		fd = shm_open(MEM_NAME, O_RDWR, 0644);
		fstat(fd, &buf);
		size = buf.st_size;
		protection = PROT_READ;
	}
	else {
		ftruncate(fd, sizeof(struct shared_mem));
		size = sizeof(struct shared_mem);
		protection = PROT_READ | PROT_WRITE;
	}

	printf("\nMapping Shared mem\n\n");
	if((sharedptr = mmap(NULL, size, protection, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		fprintf(stderr, "Error in mmap in reader => %s\n\n", strerror(errno));
		return -1;
	}
	close(fd);

	if(exists != 1) {
		printf("\nExists 2\n");
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
                fprintf(stderr, "Error in munmap in reader => %s", strerror(errno));
                return;
        }
        if(shm_unlink(MEM_NAME) == -1) {
                fprintf(stderr, "Error in shm_unlink in reader => %s", strerror(errno));
                return;
        }
}

int
main(void) {

	atexit(delete_shared_mem);
	if(create_shared_mem() == -1) 
		goto exit;
	if(read_from_shared_mem() == -1)
		goto exit;

exit:
	return 0;
}
