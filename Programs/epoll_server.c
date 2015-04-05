#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h> 
#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>

struct _client {
	int fd;
	char *name;
	struct _client *peer;
	struct _client *next;
};

typedef int (EPOLL_CTL_HANDLER) (void *data);
struct _ectl {
	EPOLL_CTL_HANDLER *handler;
};

#define log_msg(term, msg, arg...) fprintf(term, msg, ##arg)
#define log_err(msg, arg...) log_msg(stderr, msg, ##arg)
#define log_info(msg, arg...) log_msg(stdout, msg, ##arg)
#define log_client_msg(log_func, ptr, msg, arg...) do {		\
	    if (ptr->name) {					\
		    log_func("client %s: " msg, ptr->name, ##arg);	\
	    } else {						\
		    log_func("client %d: " msg, ptr->fd, ##arg);	\
	    }								\
	} while(0)
#define log_client_err(ptr, msg, arg...) log_client_msg(log_err, ptr, msg, ##arg)
#define log_client_info(ptr, msg, arg...) log_client_msg(log_info, ptr, msg, ##arg)
#define MAX_EVENTS 256
#define ECTL_SIZE sizeof(struct _ectl)
#define PTR2ECTL(__ptr) (struct _ectl *) (((uint8_t *)__ptr) - ECTL_SIZE);
#define ECTL2PTR(__ectl) (((uint8_t *)__ectl) + ECTL_SIZE)
#define ECTL2INT(__ectl) (int *)ECTL2PTR(__ectl)
#define ECTL2CLIENT(__ectl) (struct _client *)ECTL2PTR(__ectl)

#define send_data(__cnode, __buf, __len) do {					\
		if (send(__cnode->fd, __buf, __len, 0) < 0) {			\
			log_client_err(__cnode, "send(%d) failed '%m'\n", __cnode->fd);	\
			remove_client(__cnode);					\
			return -1;						\
		}								\
	} while(0)
#define send_error(__cnode, __buf) send_data(__cnode, __buf, strlen(__buf))

static int cmd_ident(struct _client *, const char *);
static int cmd_help(struct _client *, const char *);
static int cmd_connect(struct _client *, const char *);
static int cmd_disconnect(struct _client *, const char *);
static int cmd_close(struct _client *, const char *);
typedef int (CMD_HANDLER) (struct _client *, const char *);
struct _command {
	const char *cmd;
	int len;
	CMD_HANDLER *handler;
} cmd[] = {
	{"ident ",	6, cmd_ident},
	{"connect ",	8, cmd_connect},
	{"disconnect ",	11, cmd_disconnect},
	{"close ",	6, cmd_close},
	{"help ",	5, cmd_help},
	{NULL, 		0, NULL}
};
static int exit_global;
struct _ectl *fde;
struct _client *clist;
int epollfd;

static void 
sigint(int signal) {
	(void) signal;
	exit_global = 1;
}

static struct _client *
get_client(void)
{
	struct _client *node;
	uint8_t *ptr;

	ptr = calloc(1, ECTL_SIZE + sizeof(struct _client));
	node = ECTL2CLIENT(ptr);
	if (!clist) {
		clist = node;
	} else {
		node->next = clist;
		clist = node;
	}
	return node;
}

static int
remove_ectl(struct _ectl *ectl)
{
	return 0;
}

static int
remove_client(struct _client *cnode)
{
	return 0;
}

static inline int
register_ectl(int fd, EPOLL_CTL_HANDLER *handler, struct _ectl *ectl)
{
	struct epoll_event epv;

	ectl->handler = handler;
	memset(&epv, 0, sizeof epv);
	epv.events = EPOLLIN;
	epv.data.ptr = ectl;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &epv) < 0) {
		log_err("epoll_ctl(%d) failed for %d:%m\n", epollfd, fd);
		remove_ectl(ectl);
		return -1;
	}
	return 0;
}

static inline int
do_relay(struct _client *src, struct _client *dst)
{
	int nread;
	char buf[2048];

	if ((nread = recv(src->fd, buf, sizeof(buf), 0)) > 0) {
		send_data(dst, buf, nread);
		return 0;
	}
	if (nread == 0) {
		log_client_err(src, "closed.");
	} else {
		log_client_err(src, "recv(%d) failed '%m'.\n", src->fd);
	}
	remove_client(src);
	return 0;
}

static int
_client_event(struct _client *cnode, const char *buf)
{
	register uint32_t i;
	if (strncasecmp(buf, "cmd ", 4) == 0) {
		buf+=4;
		for (i=0; cmd[i].cmd; i++) {
			if (strncasecmp(cmd[i].cmd, buf, cmd[i].len) == 0) {
				return cmd[i].handler(cnode, buf);
			}
		}
		send_error(cnode, "Invalid command. 'cmd help' for more info");
		return 0;
	}
	if (!cnode->peer) {
		send_error(cnode, "Invalid command OR first connect to another clinet to send data. 'cmd help' for more info");
		return 0;
	}
	return do_relay(cnode, cnode->peer);
}

static int
client_event(void *data)
{
	char buf[4096];
	struct _client *cnode = data;
	int nrecv;

	if ((nrecv = recv(cnode->fd, buf, sizeof(buf)-1, 0)) > 0) {
		buf[nrecv] = '\0';
		return _client_event(cnode, buf);
	}
	if (nrecv == 0) {
		remove_client(cnode);
		return 0;
	}
	log_client_err(cnode, "recv(%d) failed %m", cnode->fd);
	return -1;
}

static inline int
register_client(int fd)
{
	struct _client *cnode;
	struct _ectl *ectl;

	cnode = get_client();
	cnode->fd = fd;
	ectl = PTR2ECTL(cnode);
	return register_ectl(fd, client_event, ectl);
}

static int
server_event(void *data)
{
	int connfd, sockfd=*((int*)data);
	struct sockaddr_in client_sock;
	socklen_t len;

	memset(&client_sock, 0, sizeof(client_sock));
	len = sizeof(client_sock);
	
	if((connfd = accept(sockfd, (struct sockaddr *)&client_sock, &len)) < 0) {
		log_err("accept(%d) failed '%m'\n", sockfd);
		return -1;
	}
	if (register_client(connfd) < 0) {
		return -1;
	}
	log_info("client %d registered\n", connfd);
	return 0;
}

static inline int
cmd_ident(struct _client *cnode, const char *buf)
{
	buf += 6;
	if (strlen(buf) == 0) {
		send_error(cnode, "null ident is not allowed");
		return 0;
	}
	if (cnode->name) {
		free(cnode->name);
	}
	cnode->name = strdup(buf);
	return 0;
}

static inline int
cmd_connect(struct _client *cnode, const char *buf)
{
	return 0;
}

static inline int
cmd_help(struct _client *cnode, const char *buf)
{
	return 0;
}

static inline int
cmd_disconnect(struct _client *cnode, const char *buf)
{
	return 0;
}

static inline int
cmd_close(struct _client *cnode, const char *buf)
{
	return 0;
}

static void
do_cleanup(void)
{
}

static int
do_polling(void)
{
	int ret = -1, i, nfd;	
	struct epoll_event events[MAX_EVENTS];
	struct _ectl *ectl;
	
	while(!exit_global) {
		if((nfd = epoll_wait(epollfd, events, MAX_EVENTS, -1)) < 0) {
			if (errno == EINTR) {
				continue;
			}
			log_err("epoll_wait(%d) failed %m", epollfd);
			goto clean_up;
		}
		for(i=0; i<nfd; i++) {
			ectl = events[i].data.ptr;
			if (ectl->handler(ECTL2PTR(ectl)) < 0) {
				goto clean_up;
			} 
		}
	}
	ret = 0;
clean_up:
	do_cleanup();
	return ret;
}

static inline int
create_socket(void)
{
	int sockfd, opt=1, *eint;
	struct sockaddr_in server_sock;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		log_err("socket() failed '%m'");
		return -1;
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		log_err("setsockopt(%d) failed '%m'\n", sockfd);
		close(sockfd);
		return -1;
	}
	memset(&server_sock, 0, sizeof(server_sock));
	server_sock.sin_family = AF_INET;
	server_sock.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sock.sin_port = htons(3000);
	if(bind(sockfd, (struct sockaddr *)&server_sock, sizeof(server_sock)) < 0) {
		log_err("bind(%d) failed %m", sockfd);
		close(sockfd);
		return -1;
	}
	if(listen(sockfd, 10) < 0) {
		log_err("listen(%d) failed %m\n", sockfd);
		close(sockfd);
		return -1;
	}
	fde=calloc(1, ECTL_SIZE+sizeof(int));
	eint = ECTL2INT(fde);
	*eint = sockfd;
	if (register_ectl(sockfd, server_event, fde) < 0) {
		close(sockfd);
		return -1;
	}
	return sockfd;
}

int
main(void)
{
	signal(SIGINT, sigint);
		
	if((epollfd = epoll_create(10)) < 0) {
		log_err("epoll_create() error '%m'");
		return -1;
	}
	if (create_socket() < 0) {
		close(epollfd);
		return -1;
	}
	log_info("Server started successfully, waiting for client...\n");
	return do_polling();
}
