
#include "shared.h"
#include<stdlib.h>

static struct shared_mem *sharedptr;

static int
read_from_shared_mem() {
	int i, value;
	printf("\n In read");
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
get_shared_mem() {
	int fd;
	printf("\n Accessing Shared mem");
	if((fd = shm_open(MEM_NAME, O_RDONLY, 0644)) == -1) {
		fprintf(stderr, "Error in creating shared memory in reader => %s", strerror(errno));
		return -1;
	}
	ftruncate(fd, sizeof(struct shared_mem));
	printf("\nMapping Shared mem");
	if((sharedptr = mmap(NULL, sizeof(struct shared_mem), PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		fprintf(stderr, "Error in mmap in reader => %s", strerror(errno));
		return -1;
	}
	close(fd);
	return 0;
}

int
main(void) {

	if(get_shared_mem() == -1) 
		goto exit;
	if(read_from_shared_mem() == -1)
		goto exit;

exit:
	return 0;
}
