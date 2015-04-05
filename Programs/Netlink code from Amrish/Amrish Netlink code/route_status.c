#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include "netlink_common.h"

/* I am defining following macro because latest kernel near 2.6.28 fills follwing 
 * rta attribute but header files are no updated accordingly. so I have added it
 * manually. I will remove if when in future this macri will be availble in rtnetlink.h
 */
#define RTA_TABLE 15

static inline void
display_dst(struct rtmsg *rg, void *data)
{
    uint8_t *dt = data;
    log_info_append("%d.%d.%d.%d/%d ",dt[0], dt[1], dt[2], dt[3], rg->rtm_dst_len);
    log_info_append("[%2X %2X %2X %2X]", dt[7], dt[6], dt[5], dt[4]);
}

static inline void
display_src(struct rtmsg *rg, void *data)
{
    uint8_t *dt = data;
    log_info_append("%d.%d.%d.%d/%d ",dt[0], dt[1], dt[2], dt[3], rg->rtm_src_len);
    log_info_append("[%2X %2X %2X %2X]", dt[7], dt[6], dt[5], dt[4]);
}

static inline void
display_prefsrc(void *data)
{
    uint8_t *dt = data;
    log_info_append("%d.%d.%d.%d ",dt[0], dt[1], dt[2], dt[3]);
    log_info_append("[%2X %2X %2X %2X]", dt[7], dt[6], dt[5], dt[4]);
}

static inline void
display_gateway(void *data)
{
    uint8_t *dt = data;
    log_info_append("%d.%d.%d.%d ",dt[0], dt[1], dt[2], dt[3]);
    log_info_append("[%2d %2d %2d %2d]", dt[7], dt[6], dt[5], dt[4]);
}

static inline void
handle_route_attribute(struct rtmsg *rg, struct rtattr *rt, void *data)
{
    switch (rt->rta_type) {
	case RTA_UNSPEC:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_UNSPEC", rt->rta_len);
	    break;
	case RTA_DST:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_DST", rt->rta_len);
	    display_dst(rg, data);
	    break;
	case RTA_SRC:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_SRC", rt->rta_len);
	    display_src(rg, data);
	    break;
	case RTA_IIF:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_IIF", rt->rta_len);
	    display_raw(rt->rta_len, data);
	    break;
	case RTA_OIF:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_OIF", rt->rta_len);
	    display_raw(rt->rta_len, data);
	    break;
	case RTA_GATEWAY:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_GATEWAY", rt->rta_len);
	    display_gateway(data);
	    break;
	case RTA_PRIORITY:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_PRIORITY", rt->rta_len);
	    display_raw(rt->rta_len, data);
	    break;
	case RTA_PREFSRC:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_PREFSRC", rt->rta_len);
	    display_prefsrc(data);
	    break;
	case RTA_METRICS:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_METRICS", rt->rta_len);
	    display_raw(rt->rta_len, data);
	    break;
	case RTA_MULTIPATH:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_MULTIPATH", rt->rta_len);
	    display_raw(rt->rta_len, data);
	    break;
	case RTA_PROTOINFO:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_PROTOINFO", rt->rta_len);
	    display_raw(rt->rta_len, data);
	    break;
	case RTA_FLOW:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_FLOW", rt->rta_len);
	    display_raw(rt->rta_len, data);
	    break;
	case RTA_CACHEINFO:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_CACHEINFO", rt->rta_len);
	    display_raw(rt->rta_len, data);
	    break;
	case RTA_SESSION:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_SESSION", rt->rta_len);
	    display_raw(rt->rta_len, data);
	    break;
	case RTA_MP_ALGO:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_MP_ALGO", rt->rta_len);
	    display_raw(rt->rta_len, data);
	    break;
	case RTA_TABLE:
	    log_info_append("TYPE(%-15s) LEN(%2d)", "RTA_TABLE", rt->rta_len);
	    display_raw(rt->rta_len, data);
	    break;
	default:
	    log_info_append("Undefined type(%d) LEN(%2d)", rt->rta_type, rt->rta_len);
	    display_raw(rt->rta_len, data);
    }
    log_info_append("\n");
}

static inline void
display_rtm_protocol(unsigned char proto)
{
    log_info_append("RTM_PROTOCOL(");
    switch (proto) {
	case RTPROT_UNSPEC:
	    log_info_append("RTPROT_UNSPEC");
	    break;
	case RTPROT_REDIRECT:
	    log_info_append("RTPROT_REDIRECT");
	    break;
	case RTPROT_KERNEL:
	    log_info_append("RTPROT_KERNEL");
	    break;
	case RTPROT_BOOT:
	    log_info_append("RTPROT_BOOT");
	    break;
	case RTPROT_STATIC:
	    log_info_append("RTPROT_STATIC");
	    break;
	case RTPROT_GATED:
	    log_info_append("RTPROT_GATED");
	    break;
	case RTPROT_RA:
	    log_info_append("RTPROT_RA");
	    break;
	case RTPROT_MRT:
	    log_info_append("RTPROT_MRT");
	    break;
	case RTPROT_ZEBRA:
	    log_info_append("RTPROT_ZEBRA");
	    break;
	case RTPROT_BIRD:
	    log_info_append("RTPROT_BIRD");
	    break;
	case RTPROT_DNROUTED:
	    log_info_append("RTPROT_DNROUTED");
	    break;
	case RTPROT_XORP:
	    log_info_append("RTPROT_XORP");
	    break;
	case RTPROT_NTK:
	    log_info_append("RTPROT_NTK");
	    break;
	default :
	    log_info_append("Undefined protocol '%X'\n", proto);
    }
    log_info_append(")==");
}

static inline void
display_rtm_family(unsigned char family)
{
    log_info_append("RTM_FAMILY(");
    switch (family) {
	case AF_INET:
	    log_info_append("AF_INET");
	    break;
	case AF_INET6:
	    log_info_append("AF_INET6");
	    break;
	case AF_DECnet:
	    log_info_append("AF_DECnet");
	    break;
	default :
	    /* Please fill unknows family type from linux/socket.h */
	    log_info_append("Unknown family type %X", family);
    }
    log_info_append(")==");
}

static inline void
display_rtm_table(unsigned char tbl)
{
    log_info_append("ROUTING_TABLE(");
    switch (tbl) {
	case RT_TABLE_UNSPEC:
	    log_info_append("RT_TABLE_UNSPEC");
	    break;
	case RT_TABLE_DEFAULT:
	    log_info_append("RT_TABLE_DEFAULT");
	    break;
	case RT_TABLE_MAIN:
	    log_info_append("RT_TABLE_MAIN");
	    break;
	case RT_TABLE_LOCAL:
	    log_info_append("RT_TABLE_LOCAL");
	    break;
	default:
	    log_info_append("User Defined %d", tbl);
    }
    log_info_append(")==");
}

static inline void
display_rtm_scope(unsigned char scop)
{
    log_info_append("RTM_SCOPE(");
    switch (scop) {
	case RT_SCOPE_UNIVERSE:
	    log_info_append("RT_SCOPE_UNIVERSE");
	    break;
	case RT_SCOPE_SITE:
	    log_info_append("RT_SCOPE_SITE");
	    break;
	case RT_SCOPE_LINK:
	    log_info_append("RT_SCOPE_LINK");
	    break;
	case RT_SCOPE_HOST:
	    log_info_append("RT_SCOPE_HOST");
	    break;
	case RT_SCOPE_NOWHERE:
	    log_info_append("RT_SCOPE_NOWHERE");
	    break;
	default:
	    log_info_append("User defined %X", scop);
    }
    log_info_append(")");
}

static inline void
display_rtm_type(unsigned char type)
{
    log_info_append("RTM_TYPE(");
    switch (type) {
	case RTN_UNSPEC:
	    log_info_append("RTN_UNSPEC");
	    break;
	case RTN_UNICAST:
	    log_info_append("RTN_UNICAST");
	    break;
	case RTN_LOCAL:
	    log_info_append("RTN_LOCAL");
	    break;
	case RTN_BROADCAST:
	    log_info_append("RTN_BROADCAST");
	    break;
	case RTN_ANYCAST:
	    log_info_append("RTN_ANYCAST");
	    break;
	case RTN_MULTICAST:
	    log_info_append("RTN_MULTICAST");
	    break;
	case RTN_BLACKHOLE:
	    log_info_append("RTN_BLACKHOLE");
	    break;
	case RTN_UNREACHABLE:
	    log_info_append("RTN_UNREACHABLE");
	    break;
	case RTN_PROHIBIT:
	    log_info_append("RTN_PROHIBIT");
	    break;
	case RTN_THROW:
	    log_info_append("RTN_THROW");
	    break;
	case RTN_NAT:
	    log_info_append("RTN_NAT");
	    break;
	case RTN_XRESOLVE:
	    log_info_append("RTN_XRESOLVE");
	    break;
	default:
	    log_info_append("Undefined type %X", type);
    }
    log_info_append(")==\n   ");
}

static inline void
display_rtm_flags(unsigned flags)
{
    log_info_append("\n   RTM_FLAGS(%X){ ", flags);
    if (flags & RTM_F_NOTIFY)		log_info_append("RTM_F_NOTIFY ");
    if (flags & RTM_F_CLONED)		log_info_append("RTM_F_CLONED ");
    if (flags & RTM_F_EQUALIZE)		log_info_append("RTM_F_EQUALIZE ");
    if (flags & RTM_F_PREFIX)		log_info_append("RTM_F_PREFIX ");
    log_info_append("\b}");
}

static inline void
display_rtmsg(struct rtmsg *rg)
{
    log_info_append("\n===");
    display_rtm_family(rg->rtm_family);
    display_rtm_table(rg->rtm_table);
    display_rtm_protocol(rg->rtm_protocol);
    display_rtm_type(rg->rtm_type);
    log_info_append("RTM_DST_LEN(%d) ", rg->rtm_dst_len);
    log_info_append("RTM_SRC_LEN(%d) ", rg->rtm_src_len);
    log_info_append("RTM_TOS(%d) ", rg->rtm_tos);
    display_rtm_scope(rg->rtm_scope);
    display_rtm_flags(rg->rtm_flags);
    log_info_append("\n----------------\n");
}

static inline int
send_route_get_request(int fd)
{
    struct sockaddr_nl nl;
    struct _nl_route_req req;
    struct iovec iov;
    struct msghdr msg = {(void*)&nl, sizeof nl, &iov, 1, NULL, 0, 0};

    memset(&nl, 0, sizeof nl);
    nl.nl_family = AF_NETLINK;

    memset(&req, 0, sizeof req);
    /* certain documents suggest to fill follwing item with getpid(), but that is not necessary
     * req.nh.nlmsg_pid = getpid();
     */
    req.nh.nlmsg_type = RTM_GETROUTE;
    req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));

    iov.iov_base = &req;
    iov.iov_len = req.nh.nlmsg_len;

    if (sendmsg(fd, &msg, 0) == -1) {
	log_critical("sendmsg(%d) failed '%s'\n", fd, strerror(errno));
	return -1;
    }
    return 0;
}

static inline int
is_valid_route_nh(struct nlmsghdr *nh)
{
    if (nh->nlmsg_type != RTM_NEWROUTE &&
	nh->nlmsg_type != RTM_DELROUTE   ) {
	log_critical("Invalid nlmsgtype %X\n", nh->nlmsg_type);
	return -1;
    }
    return 0;
}

static inline int
route_attr_hdlr(struct nlmsghdr *nh, void *subobj, int rtlen)
{
    struct rtmsg *rtobj = subobj;
    struct rtattr *rt;

    if (is_valid_route_nh(nh) < 0) {
	return -1;
    }
    display_rtmsg(rtobj);
    rt = RTM_RTA(rtobj);
    if (!RTA_OK(rt, rtlen)) {
	log_notice("RTA_OK failed\n");
	return -1;
    }
    for (;RTA_OK(rt, rtlen); rt = RTA_NEXT(rt, rtlen)) {
	handle_route_attribute(rtobj, rt, RTA_DATA(rt));
    }
    return 0;
}

static inline int
get_routing_info(int fd)
{
    char rb[4096];
    int len;
    int ret;

    if (send_route_get_request(fd) == -1) {
	return -1;
    }
    if ((len=receive_netlink_response(rb, 4096, fd)) == -1) {
	return -1;
    }
    if (parse_netlink_response(rb, 4096, route_attr_hdlr, &ret) < 0) {
	return -1;
    }
    return 0;
}

static inline int
display_route_info(void)
{
    int fd;
    if ((fd = init_rtnetlink_socket(0)) < 0) {
	return -1;
    }
    if (get_routing_info(fd) < 0) {
	return -1;
    }
    return 0;
}

static inline int
prepare_request(struct _nl_route_req *req)
{
    memset(req, 0, sizeof(*req));

    req->nh.nlmsg_type = RTM_GETROUTE;
    req->nh.nlmsg_flags = NLM_F_REQUEST;
    req->nh.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));

    req->route.rtm_family = AF_INET;
    return 0;
}

static inline int
add_rta_dst_attr(struct _nl_route_req *req, const int siz, const char *ip)
{
    struct in_addr add;
    struct rtattr *rta;
    int len = RTA_LENGTH(sizeof add.s_addr);
    int req_len = NLMSG_ALIGN(req->nh.nlmsg_len) + RTA_ALIGN(len);

    if (req_len > siz) {
	log_error("not enough space in structure\n");
	return -1;
    }

    if (inet_pton(AF_INET, ip, &add) < 0) {
	log_error("inet_pton failed '%s'\n", strerror(errno));
	return -1;
    }
    rta = NLMSG_TAIL(&req->nh);
    rta->rta_type = RTA_DST;
    rta->rta_len = len;
    memcpy(RTA_DATA(rta), &(add.s_addr), sizeof add.s_addr);
    req->nh.nlmsg_len = req_len;
    return req_len;
}

static inline int
get_ip_route_info(int argc, char **argv)
{
    int fd;
    char *ip;
    struct _nl_route_req req;
    char rb[4096];
    int len, ret;

    if (argc < 2) {
	return 0;
    }
    ip = argv[1];
    if ((fd = init_rtnetlink_socket(0)) < 0) {
	return -1;
    }
    if (prepare_request(&req) < 0) {
	goto clean_up;
    }
    if (add_rta_dst_attr(&req, sizeof req, ip) < 0) {
	goto clean_up;
    }
    if (send_netlink_msg(fd, &req) < 0) {
	goto clean_up;
    }
    if ((len=receive_netlink_response(rb, 4096, fd)) == -1) {
	return -1;
    }
    if (parse_netlink_response(rb, 4096, route_attr_hdlr, &ret) < 0) {
	return -1;
    }
    return 0;
clean_up:
    close(fd);
    return -1;
}

int
main(int argc, char **argv)
{
 /*   if (display_route_info() < 0) {
	return -1;
    }*/
    if (get_ip_route_info(argc, argv) < 0) {
	return -1;
    }
    return 0;
}
