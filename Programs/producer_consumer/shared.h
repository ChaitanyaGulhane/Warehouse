
#ifndef SHARED_H
#define SHARED_H

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<semaphore.h>
#include<errno.h>
#include<sys/mman.h>
#include<string.h>

#define MAX 16
#define MEM_NAME "Producer_Consumer"

struct data {
	int id;
	char name[32];
};

struct shared_mem {
	sem_t nempty;
	sem_t nstored;
	sem_t mutex;
	struct data data[MAX]; 		
};

#endif
