#ifndef __NETLINK_COMMON__
#define __NETLINK_COMMON__

#include<sys/socket.h>
#include<linux/netlink.h>
#include<linux/rtnetlink.h>

#ifndef __LOG_H__
#define log_temp(fd, arg...)            fprintf(fd, arg)
#define log_debug(str, arg...)          log_temp(stdout, "%s:"str, __FUNCTION__, ##arg)
#define log_error(str, arg...)          log_temp(stderr, "%s:"str, __FUNCTION__, ##arg)
#define log_info(str, arg...)           log_temp(stdout, "%s:"str, __FUNCTION__, ##arg)
#define log_notice(str, arg...)         log_temp(stderr, "%s:"str, __FUNCTION__, ##arg)
#define log_message(str, arg...)        log_temp(stdout, "%s:"str, __FUNCTION__, ##arg)
#define log_critical(str, arg...)       log_temp(stderr, "%s:"str, __FUNCTION__, ##arg)

#define log_info_append(arg...)		log_temp(stdout, arg)
#endif

#define NLMSG_TAIL(nmsg) \
            ((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

#define ROUTE_ATTR_SIZE 	1024
typedef int (NETLINK_PARSER) (struct nlmsghdr *nh, void *sub_object, int rtlen);
//typedef int (NETLINK_PARSER) (struct nlmsghdr *nh, void *sub_object, int rtlen, void *data);
struct _nl_route_req {
    struct nlmsghdr  nh;
    struct rtmsg     route;
    char 	     rt_attr[ROUTE_ATTR_SIZE];
};

void display_raw(const int len, uint8_t *ptr);
int send_netlink_msg(int fd, void *req);
int parse_netlink_response(const char *buf, uint32_t len, NETLINK_PARSER *netparser, int *hnd_ret);
int receive_netlink_response(char *buf, int len, int fd);
int init_rtnetlink_socket(uint32_t group);
#endif
