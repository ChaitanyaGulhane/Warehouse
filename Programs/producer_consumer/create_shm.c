
#include "shared.h"
#include<string.h>

struct shared_mem *sharedptr;

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
	printf("Shared mem getting created\n");
	if((fd = shm_open(MEM_NAME, O_CREAT | O_RDWR | O_EXCL, 0644)) == -1) {
		fprintf(stderr, "Error in creating shared memory in writer => %s", strerror(errno));
		return -1;
	}
	ftruncate(fd, sizeof(struct shared_mem));
	printf("Shared mem created \n");
	if((sharedptr = mmap(NULL, sizeof(struct shared_mem), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		fprintf(stderr, "Error in mmap in writer => %s", strerror(errno));
		return -1;
	}
		
	printf("Mapped shared mem\n");
	close(fd);
	if(init_sems() == -1) 
		return -1;

	return 0;
}

int
main(void) {

	create_shared_mem();
	return 0;
}
