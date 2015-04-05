#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <errno.h>

#define MAX_LEN 8192			// This buffer size should be large.

// Setup the local address to bind to that.
#define INIT_NETLINK_SOCK(x) 				\
	memset((void *)&x, 0, sizeof(x));		\
	x.nl_family = AF_NETLINK;			\
	x.nl_pid    = getpid();				\
	x.nl_groups = 0;				\


/*Used following structures below

struct nlmsghdr {
    __u32 nlmsg_len;    // Length of message including header. 
    __u16 nlmsg_type;   // Type of message content. 
    __u16 nlmsg_flags;  // Additional flags. 
    __u32 nlmsg_seq;    // Sequence number. 
    __u32 nlmsg_pid;    // Sender port ID. 
};

- This is used for creating the message. This mesage header is required 
to create any netlink socket message to kernel.


struct rtgenmsg {
      unsigned char  rtgen_family;
};

- This is used 

*/
struct _request {
	struct nlmsghdr nh;
	struct rtgenmsg gen;
};

/*
Used following structure below:

struct msghdr {
	void         *msg_name        // optional address
	socklen_t     msg_namelen     // size of address
	struct iovec *msg_iov         // scatter/gather array
	int           msg_iovlen      // members in msg_iov
	void         *msg_control     // ancillary data - used only related to struct cmsg
	socklen_t     msg_controllen  // ancillary data - used only when cmsg header is used
	int           msg_flags       // flags on received message
}

- This is used to put the header in any messgae sent by sentmsg().

struct iovec {
      void  *iov_base;    // Starting address 
      size_t iov_len;     // Number of bytes to transfer 
};

- This is used to supply input/output vector in msghdr.

*/
struct _msg {
	struct msghdr msg;
	struct iovec io;
};

struct sockaddr_nl nl;

static inline 
int open_netlink_socket(int *socket_netlink) {
	int ret = 0;
	if(((*socket_netlink) = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0 ) {
		fprintf(stderr, "socket() error => %m\n");
		ret = *socket_netlink;	
	}
	return ret;
}

static inline 
int bind_netlink_socket(const int socket_netlink, const struct sockaddr_nl *sock_addr) {
	int ret = 0;
	if((ret = bind(socket_netlink, (struct sockaddr *)sock_addr, sizeof(struct sockaddr_nl))) < 0 ) {
		fprintf(stderr, "bind(%d) error =. %m\n", socket_netlink);
	}
	return ret;
}

static inline
void prepare_request(struct _request *req) {

	memset(req, 0 , sizeof(*req));

	//Preparing nlmsghdr 
	req->nh.nlmsg_type = RTM_GETLINK;				// Getting address 
	req->nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;		// The type of message is request and (Get the root OR Get the matching tree)
	req->nh.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));	// Length of rtgenmsg
	req->nh.nlmsg_seq = 1;						// Indicates first message
	req->nh.nlmsg_pid = getpid();					// Indicates the pid 
	
	req->gen.rtgen_family = AF_PACKET;				// AF_PACKET is used to get all link information inspite of only IP related links
	
}

static inline 
int send_msg_to_kernel(const int sock_netlink, struct _request *req) {

	struct _msg msg_to_kernel;

	memset((void*)&nl, 0, sizeof(struct sockaddr_nl));
	memset((void*)&msg_to_kernel, 0, sizeof(struct _msg));

	nl.nl_family = AF_NETLINK;

	msg_to_kernel.io.iov_base = req;
	msg_to_kernel.io.iov_len = req->nh.nlmsg_len;
	msg_to_kernel.msg.msg_name = &nl;
	msg_to_kernel.msg.msg_namelen = sizeof(nl);
	msg_to_kernel.msg.msg_iov = &msg_to_kernel.io;
	msg_to_kernel.msg.msg_iovlen = 1;

	if(sendmsg(sock_netlink, (struct msghdr *)&msg_to_kernel.msg, 0) < 0) {
		fprintf(stderr, "sendmsg(%d) error => %d - %s\n", sock_netlink, errno, strerror(errno));
		return -1;
	}

	fprintf(stdout, "sendmsg success..!!\n");
	return 0;
}

static inline
void print_link(struct nlmsghdr *nlmsg) {

	struct ifinfomsg *ifname = NLMSG_DATA(nlmsg);
	int len = nlmsg->nlmsg_len - NLMSG_LENGTH(sizeof(*ifname));
	struct rtattr *attr;

	for(attr = IFLA_RTA(ifname); RTA_OK(attr, len); attr = RTA_NEXT(attr, len)) {
		switch(attr->rta_type) {
			case IFLA_IFNAME:
				fprintf(stdout, "Interface %d : %s\n", ifname->ifi_index, (char *) RTA_DATA(attr));
			break;
			
			default:
			break;
		}
	}	
	
}

static inline
int parse_msg(const char* buff, int len) {
	struct nlmsghdr *nlmsg;
	for(nlmsg = (struct nlmsghdr* )buff; NLMSG_OK(nlmsg, len); nlmsg = NLMSG_NEXT(nlmsg, len)) {
		switch(nlmsg->nlmsg_type) {
			case NLMSG_DONE:
				print_link(nlmsg);	
				fprintf(stdout, "End of message \n");
			return 0;

			case RTM_NEWLINK:
				print_link(nlmsg);	
			break;
			
			default:
				print_link(nlmsg);	
				fprintf(stdout, "message type %d, length %d\n", nlmsg->nlmsg_type, nlmsg->nlmsg_len);
			break;
		}
	}
	return -1;
}

static inline
int recv_msg_from_kernel(int sock_netlink) {
	
	int ret = 0;
	char buff[MAX_LEN];
	struct _msg msg_from_kernel;

	memset((void*)&msg_from_kernel, 0, sizeof(struct _msg));
	msg_from_kernel.io.iov_base = buff;
	msg_from_kernel.io.iov_len = MAX_LEN;
	msg_from_kernel.msg.msg_name = &nl;
	msg_from_kernel.msg.msg_namelen = sizeof(nl);
	msg_from_kernel.msg.msg_iov = &msg_from_kernel.io;
	msg_from_kernel.msg.msg_iovlen = 1;

	while(1) {
		if((ret = recvmsg(sock_netlink, &msg_from_kernel.msg, 0)) < 0) {
			fprintf(stderr, "recvmsg(%d) err => %m\n", sock_netlink);
			break;
		}
		if(parse_msg(buff, ret) == 0) {
			break;
		}
	}	
	return ret;
}

int main() {
	int socket_netlink = 0;
	int ret = 0;
	struct sockaddr_nl sock_addr;
	struct _request req;
	if((ret = open_netlink_socket(&socket_netlink)) <  0) {
		goto cleanup;
	}
	fprintf(stdout, "Socket opened => %d\n", socket_netlink);
	INIT_NETLINK_SOCK(sock_addr);
	if((bind_netlink_socket(socket_netlink, &sock_addr)) < 0) {
		goto close_sock;
	}
	fprintf(stdout, "bind(%d) successfull\n", socket_netlink);
	prepare_request(&req);
	
	fprintf(stdout, "request prepared\n");
	if(send_msg_to_kernel(socket_netlink, &req) < 0) {
		goto close_sock;
	}
	fprintf(stdout, "send msg done\n");
	if(recv_msg_from_kernel(socket_netlink)  < 0) {
		goto close_sock;
	}
	fprintf(stdout, "recv msg done\n");
close_sock:
	close(socket_netlink);	
	fprintf(stdout, "closed socket\n");
cleanup:
	return ret;
}
