#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<sys/ipc.h>

#define MAX_BUFFER 4096

void sharedmemory(void)
{
	int fork_flag;
	pid_t c_pid, p_pid;
	int shm_id;
	key_t key  = 1234;
	char *buf;	


	if((fork_flag = fork()) == 0)
	{
		pid_t c_pid = getpid();		
	//	printf("\n\nChild Process ... PID = %d \n\n", c_pid);	

		if((shm_id = shmget(key, MAX_BUFFER, 0777)) == -1)
                {
                        perror("Error while creating a shared memory segmentin child process...!!!");
                        exit(0);
                }

		printf("Child Process --- Shanred Memory Created. \n\n");
        	      
	  	if((buf = (char *) shmat(shm_id, NULL, 0)) == (char *)-1)
                {
                        perror("shmat error in Child process...!!!");
                        exit(0);
                }
	
		printf("Child Process --- Shanred Memory attached. \n\n");
		
		strcpy(buf,"Chaitanya");
		printf("Child Process --- copy string. \n\n");
                shmdt(&buf);
		printf("Child Process --- Shanred Memory detached \n\n");
		exit(0);
	}
	else
	{	
		p_pid = getpid();

		if((shm_id = shmget(key, MAX_BUFFER, IPC_CREAT | 0777)) == -1)
        	{
                	perror("Error while creating a shared memory segment in parent process...!!!");
                	exit(0);
        	}

		printf("Parent Process --- Shanred Memory Created. \n\n");

		if((buf = (char *) shmat(shm_id, NULL, 0)) == (char *)-1)
		{
			perror("shmat error in parent process...!!!");
			exit(0);
		}
		
		printf("Parent Process --- Shanred Memory Attached. \n\n");		
		while(buf[0] != 'C')
			sleep(1);

		printf("Parent Process --- Wait over. \n\n");
		printf("buf = %s", buf);
		printf("Parent Process --- print. \n\n");
		shmdt(&buf);
		printf("Parent Process --- Shanred Memory Detached. \n\n");
		//	printf("\n\nParent Process... PID = %d\n\n", p_pid);
		
		
	}

			
}


int main(void)
{
	sharedmemory();
	return 0;
}
