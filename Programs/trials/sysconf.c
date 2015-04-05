#include<stdio.h>
#include<unistd.h>

int main(void) {
	printf("\nMQ_PRIO_MAX  = %ld \nMQ_OPEN_MAX = %ld\n", sysconf(_SC_MQ_PRIO_MAX), sysconf(_SC_MQ_OPEN_MAX));
	return 0;
}
