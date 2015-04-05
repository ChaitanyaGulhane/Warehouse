#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>

#define MAX_SIZE 1024

typedef struct _message
{
	long int msg_type;
	char buf[MAX_SIZE];	
}Message;


void message_queue(void)
{
	pid_t ppid, cpid;
	int ret;
	key_t key = 1234;
	int msg_id;
	Message send_msg, recv_msg;

	if((ret = fork()) == 0)
	{
		waitpid(getpid(), 0 , WNOHANG);

		if((msg_id = msgget(key,  0666)) < 0)
                {
                        perror("Error while getting message queue in Child Process ...");
                        exit(-1);
                }
		
		printf("\n\nChild Process ... Message queue created");

		send_msg.msg_type = 1;
		strcpy(send_msg.buf, "Chaitanya");

		if((ret = msgsnd(msg_id, (void *)&send_msg, MAX_SIZE, 0)) < 0)
		{
			perror("Error while sending message queue in Child Process ...");
                        exit(-1);

		}

		printf("\n\nChild Process ... Message sent");

		exit(0);	
	}
	else
	{
		if((msg_id = msgget(key, IPC_CREAT | 0666)) < 0)
		{
			perror("Error while creating message queue in Parent Process ...");
			exit(-1);
		}
		
		printf("\n\nParent Process ... Message queue created");

		if((ret = msgrcv(msg_id, (void *)&recv_msg, MAX_SIZE, 0,  0)) < 0)
                {
                        perror("Error while recieving message queue in Parent Process ...");
                        exit(-1);

                }

		printf("\n\nParent Process ... Message received....");
		
		printf("\n\n\nRecieved Buffer = %s", (char *)recv_msg.buf);
		
	}
}

int main(void)
{
	message_queue();
	return 0;
}
