
#include "shared.h"

static struct shared_mem *sharedptr;

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

static int
unlink_shared_mem() {
	int fd;
	printf("Creating shm\n");
	if((fd = shm_open(MEM_NAME, O_RDWR, 0644)) == -1) {	
		if(errno == EINVAL)
			fprintf(stderr, "Invalid Shared Memory referred");
		else
			fprintf(stderr, "Error in creating shared memory in writer => %s", strerror(errno));
		return -1;
	}
	ftruncate(fd, sizeof(struct shared_mem));
	printf("mmap\n");
	if((sharedptr = mmap(NULL, sizeof(struct shared_mem), PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		fprintf(stderr, "Error in mmap in writer => %s", strerror(errno));
		return -1;
	}
	
	close(fd);
	
	if(destroy_sems() == -1)
		return -1;	
	if(munmap(sharedptr, sizeof(struct shared_mem)) == -1) {
		fprintf(stderr, "Error in munmap in writer => %s", strerror(errno));
		return -1;
	}
	if(shm_unlink(MEM_NAME) == -1) {
		fprintf(stderr, "Error in shm_unlink in writer => %s", strerror(errno));
		return -1;
	}
	return 0;
}

int
main(void) {

	unlink_shared_mem();
	return 0;
}
