#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<asm/types.h>
#include<sys/socket.h>
#include<linux/rtnetlink.h>
#include<net/if.h>


int main()
{
	int sock;				// Socket 
	struct sockaddr_nl sock_addr_netlink; 	// netlink socket address
	int ret;				// return
	char buf[4096];
	int nll = 0;
	char * ptr;

	struct req 
	{
		struct nlmsghdr nlmsg; 			// netlink message header
		struct rtmsg rtm;			// Routing Table messgae structure
		char buffer[4096];			// buffer used while recieving the data
	} req;

	int len =0;				// length of the recieved data
	struct iovec iov;			// input output vector
	struct msghdr msg;			// message header
	int count;	
	struct nlmsghdr *nlp;
	struct rtmsg *rtm;
	struct rtattr *rt;
	int rtlen;
	char gws[32], dsts[24], src[24], unspec[32];
	int eth;
	
	// Creation of AF_NETLINK socket	
	if((sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1)
	{
		perror("Socket cannot be created");
		return -1;
	}
	
	
	// Initializing socket address descriptor
	memset(&sock_addr_netlink , 0 , sizeof(sock_addr_netlink));

	// Assigning values to sockaddr_nl fields
	sock_addr_netlink.nl_family = AF_NETLINK;
	sock_addr_netlink.nl_pid = 0;		// Kernel Process ID
	sock_addr_netlink.nl_groups = 0;	// Unicast

	
	//memset(nlmsg, 0 , sizeof(struct nlmsghdr));//NLMSG_SPACE(4096));
	//len = sizeof(buffer);
	
	memset(&req.nlmsg, 0 , sizeof(struct nlmsghdr));
	memset(&req.rtm, 0, sizeof(struct rtmsg));

	req.nlmsg.nlmsg_pid = getpid();					//to mark the sender's pid
	req.nlmsg.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));	// Length of the request
	req.nlmsg.nlmsg_type = RTM_GETROUTE;				// Type of the rtmsg 
	req.nlmsg.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP; 		// Flags

	
	req.rtm.rtm_family = AF_INET;					
	req.rtm.rtm_table = RT_TABLE_MAIN;
	req.rtm.rtm_type = RTN_UNICAST;

	memset(&iov, 0,sizeof(struct iovec));
	iov.iov_base = (void *) &req.nlmsg;
	iov.iov_len = req.nlmsg.nlmsg_len;
	
	memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = (void *) &sock_addr_netlink;
	msg.msg_namelen = sizeof(struct sockaddr_nl);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
					
	
	//Binding socket to the address
	if ((ret = bind(sock, (struct sockaddr *)&sock_addr_netlink, sizeof(sock_addr_netlink))) == -1)
	{
		perror("Socket cannot be bound to the address");
		return -1;
	} 
	printf("\nSocket Bound");	
	
	
	//Send and Recieve
	len =-1;	
	len = sendmsg(sock, &msg, 0);
	if(len < 0)
	{
		perror("Could not send the message");
		return -1;
	}
	printf("\n Messsage sent !!!! \n");
	
	nll = 0;	
	len = -1;
	ptr = buf;
	count =0;
	while(1)
	{
		printf("\n\n\n Count = %d", count++);
		len = recv(sock, ptr, sizeof(buf)-nll, 0);
		//len = recvmsg(sock, &ptr, 0);
		printf("\n\n\n Buf = %s", (char *)(ptr));
		//break;
		nlp = (struct nlmsghdr *) ptr;

		if(nlp->nlmsg_type == NLMSG_DONE)
	  	      break;

		printf("\n\n data = %s\nlen = %d\n", NLMSG_DATA(&req.nlmsg), len);	
		ptr = ptr + strlen(buf);
		nll = nll + len;
		
		for(; NLMSG_OK(nlp, nll); nlp = NLMSG_NEXT(nlp, nll))
		{
			rtm = (struct rtmsg *) (NLMSG_DATA(nlp));
			
			if(rtm->rtm_table != RT_TABLE_MAIN)
				continue;

			rt = (struct rtattr *) RTM_RTA(rtm);
			rtlen = RTM_PAYLOAD(nlp);
			
			memset(&gws, 0, sizeof(gws));
			memset(&dsts, 0, sizeof(dsts));
	
			for(; RTA_OK(rt, rtlen); rt = RTA_NEXT(rt, rtlen))
			{
				
				switch(rt->rta_type)
				{	
					case RTA_GATEWAY:
						inet_ntop(AF_INET, RTA_DATA(rt), gws, 32);
										
	  			        break;
					
					case RTA_DST:
          					inet_ntop(AF_INET, RTA_DATA(rt), dsts, 24);
						
				        break;
					
					case RTA_SRC:
          					inet_ntop(AF_INET, RTA_DATA(rt), src, 24);
						
				        break;

					case RTA_PRIORITY:
						printf("\n\nParameter = %s\n\n", ((char *) RTA_DATA(rt)));
				          	//inet_ntop(AF_INET, RTA_DATA(rt), unspec, 32);
					break;

					case RTA_OIF:
				          sprintf(eth, "%d", *((int *) RTA_DATA(rt)));
					break;
		
					default:
					break;
				}
					
			}
	
			printf("\n\n\n\n\n Gateway = %s", gws);	 	
			printf("\n Unspec = %s", unspec);
			printf("\n Source IP = %s", src);
			printf("\n Destination IP = %s/%d", dsts, rtm->rtm_dst_len);
			printf("\nEth = %d", eth);
			printf("\n ms = %d ", rtm->rtm_dst_len);
			break;
			
		}
		
		//if((sock_addr_netlink.nl_groups & RTMGRP_IPV4_ROUTE) == RTMGRP_IPV4_ROUTE)
 		//     break;
	}
	printf("\n\n Message Recieved...!!!!\n\n");
	
	
	
	// Closing the socket
	close(sock);
	 	 	
	return 0;
}
