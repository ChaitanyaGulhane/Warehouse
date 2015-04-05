
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
get_shared_mem() {
	int fd;
	if((fd = shm_open(MEM_NAME, O_RDWR, 0644)) == -1) {
		fprintf(stderr, "Error in creating shared memory in writer => %s", strerror(errno));
		return -1;
	}
	ftruncate(fd, sizeof(struct shared_mem));
	if((sharedptr = mmap(NULL, sizeof(struct shared_mem), PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		fprintf(stderr, "Error in mmap in writer => %s", strerror(errno));
		return -1;
	}
	close(fd);
	return 0;
}

int
main(void) {
	
	if(get_shared_mem() == -1) 
		goto exit;
	if(write_in_shared_mem() == -1)
		goto exit;

exit:
	return 0;
}
