#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/socket.h>
#include<linux/netlink.h>
#include<linux/rtnetlink.h>

#include "netlink_common.h"

void
display_raw(const int len, uint8_t *ptr)
{
    register int i;
    for (i=len-1; i>=0; i--) {
	if ((i+1)%4 == 0) {
	    log_info_append(" ");
	}
	log_info_append("%02X", ptr[i]);
    }
}

int
send_netlink_msg(int fd, void *req)
{
    int sent;
    struct sockaddr_nl nl;
    struct nlmsghdr *nh = req;
    struct iovec iov = {req, nh->nlmsg_len};
    struct msghdr msg = {(void*)&nl, sizeof nl, &iov, 1, NULL, 0, 0};

    memset(&nl, 0, sizeof nl);
    nl.nl_family = AF_NETLINK;
    if ((sent = sendmsg(fd, &msg, 0)) < 0) {
	log_critical("sendmsg(%d) failed '%s'\n", fd, strerror(errno));
	return -1;
    }
    log_info("sendmsg(%d) %d bytes sent\n", fd, sent);
    return 0;
}

static inline void
print_nlmsg_header(struct nlmsghdr *msg)
{
    log_error("printing raw data of message header may be useful to you\n");
    log_error("NLMSG_LEN(%d) NLMSG_TYPE(%d) NLMSG_FLAGS(%x) NLMSG_SEQ(%d) NLMSG_PID(%d)\n",
	    msg->nlmsg_len, msg->nlmsg_type, msg->nlmsg_flags, msg->nlmsg_seq, msg->nlmsg_pid);
}

static inline void
handle_nlmsg_error(struct nlmsgerr *err)
{
    log_critical(" may be invalid header\n");
    /* In the case of the acknowledgement error is zero */
    if (err->error < 0) {
	log_critical("NLMSG_ERROR(%s)\n", strerror(-(err->error)));
    }
    print_nlmsg_header(&err->msg);
}

/* return -2 incase of NLMSG_DONE or NLMSG_NOOP
 * 	  -1 incase of error
 * netparser	handler to call for further parsing.
 * hnd_ret	return status of the handler is filled here.
 */
int
parse_netlink_response(const char *buf, uint32_t len, NETLINK_PARSER *netparser, int *hnd_ret)
{
    uint32_t h_ret;
    struct nlmsghdr *np = (struct nlmsghdr *)buf;

    if (!NLMSG_OK(np, len)) {
	log_critical(" NLMSG_OK failed\n");
	return -1;
    }
    for (; NLMSG_OK(np, len); np=NLMSG_NEXT(np, len)) {
	if (np->nlmsg_type == NLMSG_ERROR) {
	    handle_nlmsg_error(NLMSG_DATA(np));
	    return -1;
	}
	if (np->nlmsg_type == NLMSG_OVERRUN) {
	    log_critical(" NLMSG_OVERRUN data lost\n");
	    return -1;
	}
	if (np->nlmsg_type == NLMSG_DONE) {
	    log_info(" NLMSG_DONE received\n");
	    return -2;
	}
	if (np->nlmsg_type == NLMSG_NOOP) {
	    log_info("NLMSG_NOOP received\n");
	    return -2;
	}
	if ((h_ret = netparser(np, NLMSG_DATA(np), np->nlmsg_len)) < 0) {
	    log_error("handler failed\n");
	    return -1;
	} else if (h_ret > 0) {
	    *hnd_ret = h_ret;
	    return h_ret;
	}
    }
    return 0;
}

int
receive_netlink_response(char *buf, int len, int fd)
{
    struct sockaddr_nl nl;
    struct iovec iov = {buf, len};
    struct msghdr msg = {(void*)&nl, sizeof nl, &iov, 1, NULL, 0, 0};

    if ((len = recvmsg(fd, &msg, 0)) < 0) {
	log_critical("recv(%d) failed '%s'\n", fd, strerror(errno));
	return -1;
    }
    log_info("'%d' bytes read\n", len);
    return len;
}

static inline void
prepare_nl(struct sockaddr_nl *nl, uint32_t group)
{
    memset(nl, 0, sizeof(struct sockaddr_nl));
    nl->nl_family = AF_NETLINK;
    /* This is not necessary to assign pid. If we do not
     * assign kernel will assign a unique for us see man 7 netlink
     * nl->nl_pid = getpid();
     */
    /* We are subscribing for the multicast group here */
    nl->nl_groups = group;
}

int
init_rtnetlink_socket(uint32_t group)
{
    int 	       fd;
    struct sockaddr_nl nl;

    if ((fd=socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
	log_critical("socket() failed '%s'\n", strerror(errno));
	return -1;
    }
    prepare_nl(&nl, group);
    if (bind(fd, (struct sockaddr*)&nl, sizeof(nl)) < 0) {
	log_critical("bind(%d) failed '%s'\n",
			    fd, strerror(errno));
	goto clean_up;
    }
    return fd;
clean_up:
    close(fd);
    return -1;
}

