#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/select.h>
#include<sys/time.h>
#include<arpa/inet.h> 
#include<errno.h>
#include<signal.h>
#include<sys/epoll.h>

#define SERV_PORT 3000
#define MAX_EVENTS 256
#define MAXLEN 1024
#define LISTENQ 5

#define INIT_SOCK(sock, port)\
memset(&sock, 0, sizeof(sock));\
sock.sin_family = AF_INET;\
sock.sin_addr.s_addr = htonl(INADDR_ANY);\
sock.sin_port = htons(port)\


#define log_err(arg, varg...) fprintf(stderr, arg, ##varg)
#define log_msg(arg, varg...) fprintf(stdout, arg, ##varg)

#define CMSG(msg, p)\
	if(p->name) log_msg(msg "%s\n", p->name);\
	else log_msg(msg "%d\n", p->fd) 


static int exit_global;

static void 
sigint(int signal) {
	exit_global = 1;
}

struct _client {
	int fd;
	char * name;
	struct _client *conn_client;
	struct _client *next;
};

static void 
closeall(struct _client *head) {
	struct _client *p = head, *q = NULL;
	while(p != NULL) {
		q = p;
		p = p->next;
		close(q->fd);
		q->conn_client = NULL;
		q->next = NULL;
		if(q->name)
			free(q->name);
		free(q);
	}
}

static int
free_client(struct _client **head, struct _client *p) {
	struct _client *q = (*head);
	if(q == p) {
		(*head) = p->next;		
	}
	else{
		for(; q->next != p; q = q->next);
		q->next = p->next;
	}
	close(p->fd);
	if(p->conn_client && p->conn_client->conn_client)
		p->conn_client->conn_client = NULL;
	p->conn_client = NULL;
	p->next = NULL;
	if(p->name)
		free(p->name);
	free(p);
	return 0;	
	
}

static int
client_connect(int sockfd, int epollfd,  struct _client **head) {

	int connfd;
	struct sockaddr_in client_sock;
	socklen_t len;
	struct _client *client, *p;
	struct epoll_event event;

	memset(&client_sock, 0, sizeof(struct sockaddr_in));
	len = sizeof(client_sock);
	
	if((connfd = accept(sockfd, (struct sockaddr *)&client_sock, &len)) < 0) {
		if(exit_global) {
			log_msg("\n Signal Captured -- accept() \n");
			return -1;
		}
		log_err("epoll_ctl() error => %s", strerror(errno));
		return -1;
	}
	log_msg("Client Connected => %d\n", connfd);

	memset(&event, 0, sizeof(event));
	event.events = EPOLLIN;
	event.data.fd = connfd;
	
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event) < 0) {
		log_err("epoll_ctl() error => %s", strerror(errno));
		return -1;
	}

	client = (struct _client *)malloc(sizeof(struct _client));
	client->fd = connfd;
	client->name = NULL;
	client->conn_client = NULL;
	client->next = NULL;

	if((*head) == NULL) {
		(*head) = client;
	}	
	else {
		p = (*head);
		while(p->next != NULL)
			p = p->next;
		p->next = client;
	}
	return 0;
}

static void
cmd_ident(struct _client *head, struct _client *p, char *buffer) {

	char * ptr = (buffer + 10);
	int count;
	
	if(*ptr == '\0') {
		sprintf(buffer, "Invalid Input ....!!!");
		log_msg("%s\n", buffer);
		return;
	}

	for(count = 0; count < strlen(ptr); count++) {
		if(ptr[count] == ' ') {
			sprintf(buffer, "Invalid name....!!!");
			log_msg("%s\n", buffer);
			return ;			
		}
	}
	
	if (p->name) {
		sprintf(buffer, "Client already has name =>  %s", p->name);	
		log_msg("%s\n", buffer);
		return;
	}
	struct _client *q = head;
	while(q != NULL) {
		if (q->name && (strcmp(q->name, ptr) == 0)) {
			sprintf(buffer, "The client with name = %s already exists...!!!", q->name);
			log_msg("%s\n", buffer);
			return;
		}
		q = q->next;
	}

	p->name = strdup(ptr);
	sprintf(buffer, "Client Name  = %s", p->name);
	log_msg("%s\n", buffer);
}


static int
cmd_connect(struct _client *head, struct _client *p, char * buffer) {
	
	char * ptr = (buffer + 12);
	struct _client *q = head;
	char *name;
	int len;
	
	if(*ptr == '\0') {
		sprintf(buffer, "Invalid Input");
		log_msg("%s\n", buffer);
		return -1;
	}
	
	name = strdup(ptr);
	while(q != NULL) {
		if(q->name && (strcmp(q->name, name) == 0)) {
			if(q == p) {
				sprintf(buffer, "Invalid operaion...!!!");
				log_msg("%s\n", buffer);
				break;
			}
			else {
				q->conn_client = p;
				p->conn_client = q;
				len = sprintf(buffer, " %s  <=>  %s ", p->name, q->name);	
				log_msg("%s\n", buffer);
				if(send(q->fd, buffer, len, 0) < 0) {
					log_err("send(%d) failed '%s'", q->fd, strerror(errno));
					free(name);
					return -1;
				}
				break;
			}
		}
		q = q->next;
	}
	if(q == NULL) {
		sprintf(buffer, "Client not found =>  %s ...!!!", name);
		log_msg("%s\n", buffer);
	}
	free(name);
	return 0;
}


static void
cmd_disconnect(struct _client *head, struct _client *p, char *buffer) {

	char * ptr = (buffer + 15);
	
	if(*ptr == '\0') {
		sprintf(buffer, "Invalid Input...!!!");
		log_msg("%s\n", buffer);
		return;
	}
	
	struct _client *q = NULL;
	char * name = strdup(ptr);
	q = p->conn_client;
	if(strcmp(q->name, name) == 0) {
		sprintf(buffer, " %s < != > %s", p->name, q->name);
		log_msg("%s\n", buffer);
		write(q->fd, buffer, strlen(buffer)); 
		p->conn_client = NULL;
		q->conn_client = NULL;
	}
	else if(strcasecmp(p->name, ptr) == 0){
		sprintf(buffer, "Illegal operation...!!!");
		log_msg("%s\n", buffer);
	}
	else {
		sprintf(buffer, "Client not found =>  %s ...!!!", name);
		log_msg("%s\n", buffer);
	}
	free(name);
}


static void
cmd_close(struct _client **head, struct _client *p, char * buffer) {
	free_client(head, p);
	strcpy(buffer, "Client closed");	
	log_msg("%s\n", buffer);
}

static void
cmd_commands(struct _client **head, struct _client *p,  char * buffer) {

	// For command CMD HELP
	if(strcasecmp((buffer + 4), "HELP") == 0) {
		sprintf(buffer, "\n 1.CMD IDENT _NAME_\n 2.CMD CONNECT _NAME_\n 3.CMD DISCONNECT _NAME_\n 4.CMD CLOSE");
		log_msg("%s\n", buffer);
	}
	// For command CMD IDET ABC
	else if (strncasecmp((buffer + 4), "IDENT ",6) == 0) {
		cmd_ident((*head), p, buffer);
	}
	// For command CMD CONNECT DEF
	else if((strncasecmp((buffer + 4), "CONNECT ",8) == 0) && (p->name) && (p->conn_client == NULL)) {
		cmd_connect((*head),p, buffer);
	}
	// For command CMD DISCONNECT DEF
	else if((strncasecmp((buffer + 4), "DISCONNECT ", 11) == 0) && (p->name) && (p->conn_client)) {
		cmd_disconnect((*head), p, buffer);	
	}
	// For command CMD CLOSE
	else if(strcasecmp((buffer + 4), "CLOSE") == 0) {
		cmd_close(head, p, buffer);
	}
	else {
		strcpy(buffer, "Invalid command");
		log_msg("%s\n", buffer);
	}
}

static int
do_polling(int sockfd, int epollfd) {
	
	struct epoll_event events[MAX_EVENTS];
	int i, nfd, len, tempfd;
	char buffer[1024];
	struct _client *head = NULL, *p = NULL;
	
	while(!exit_global) {
		if((nfd = epoll_wait(epollfd, events, MAX_EVENTS, -1)) < 0) {
			if (exit_global) {
				log_msg("\nSignal captured -- epoll_wait()\n");
				goto cleanup;
			}
			log_err("epoll_wait() error => %s", strerror(errno));
			goto cleanup;
		}
		for(i = 0; i < nfd; i++) {
			if(events[i].data.fd == sockfd) {
				// Connecting to client
				if (client_connect(sockfd, epollfd, &head) == -1) {
					goto cleanup;
				}
			}
			else {
				for(p = head; p != NULL; p = p->next) {
					if(events[i].data.fd == p->fd) {
						tempfd = p->fd;
						if((len = recv(p->fd, buffer, sizeof(buffer)-1, 0)) ==  0) {	
							CMSG("Client closed ", p);
							free_client(&head, p);
							break;	
						}
						if(len == -1) {
							if(exit_global) {
								log_msg("\nSignal captured -- read\n");
								goto cleanup;
							}
							log_err("recv(%d) error => %s", p->fd, strerror(errno));
							break;
						}
						buffer[len] = '\0';
					
						CMSG("\n\n******Client  => ", p);
						log_msg("\nQuery => %s \n", buffer);	
						log_msg("\nReply => ");
							
						if(strncasecmp(buffer, "CMD ", 3) == 0) {
							cmd_commands(&head, p, buffer);
						} else {
							/*We are handling data command need to check the connectivity*/
							if (p->name && p->conn_client) {
								tempfd = p->conn_client->fd;
								log_msg("%s  => From %s to %s\n", buffer, p->name, p->conn_client->name);
							}
							else {
								sprintf(buffer, "Invalid command");
								log_msg("%s\n", buffer);
							}
						}
						if((len = send(tempfd, buffer, strlen(buffer), 0)) <  0) {
							if(exit_global) {
								goto cleanup;
							}
							log_err("recv(%d) error => %s", p->fd, strerror(errno));
						}
						break;
					}
				}
			}
		}
	}

cleanup:	
	closeall(head);
	close(sockfd);
	close(epollfd);
	return 0;
}

int
main(void) {

	int sockfd, epollfd;
	struct sockaddr_in server_sock;
	struct epoll_event event;

	signal(SIGINT, sigint);
	//signal(SIGPIPE, sigpipe);
		
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		log_err("socket() error  => %s", strerror(errno));
		return -1;
	}
	INIT_SOCK(server_sock, SERV_PORT);
	
	int opt = 1;
  	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if(bind(sockfd, (struct sockaddr *)&server_sock, sizeof(server_sock)) == -1 ) {
		log_err("bind(%d) failed %s", sockfd, strerror(errno));
		close(sockfd);
		return -1;
	}
	if(listen(sockfd, LISTENQ) < 0) {
		log_err("connect() tcp error '%s'", strerror(errno));
		close(sockfd);
		return -1;
	}
	if((epollfd = epoll_create(10)) < 0) {
		log_err("epoll_create() error '%s'", strerror(errno));
		close(sockfd);
		return -1;
	}

	memset(&event, 0, sizeof(event));
	event.events = EPOLLIN;
	event.data.fd  = sockfd;
	
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event) < 0) {
		log_err("epoll_ctl() error => %s", strerror(errno));
		close(sockfd);	
		close(epollfd);
		return -1;
	}
	return do_polling(sockfd, epollfd);
}
