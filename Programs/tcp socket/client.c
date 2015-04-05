/*

Client.c

*/

#include"sock_header.h"
#include<string.h>
#define MAXLEN 20
#define SERV_PORT 3000
 
int main(int argc, char **argv)
{
	/*if(argc < 4)
	{
		perror("Arguement insufficient...!!!");
		exit(-1);
	}*/
	
	struct sockaddr_in server_sock;
	int ret, sockfd;

        if((sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		return -1;
	}

    	bzero(&server_sock, sizeof(server_sock));
    	server_sock.sin_family = AF_INET;
    	server_sock.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, argv[1], &server_sock.sin_addr);

    	if(Connect(sockfd, (struct sockaddr *)& server_sock, sizeof(server_sock)) == -1)
	{
		Close(sockfd);
		return -1;
	}

	//char *infilename = argv[2];
	//char * outfilename = argv[3];
	//FILE * infile = fopen(infilename, "r");
	//FILE * outfile = fopen(outfilename, "w");
	
	char writeline[MAXLEN];
	char readline[MAXLEN];
	int len ;
	char * buf;
	buf = (char *) malloc(sizeof(char)*strlen(argv[1]));

	//while(fgets(writeline, MAXLEN , infile) )
	//{
		Writen(sockfd, buf, strlen(buf));
		//	break;

		//bzero(readline, MAXLEN);

		//if((len = Readn(sockfd, readline, MAXLEN)) == -1)
		//	break;

		//printf("%s", readline);		

		//fputs(readline, outfile);
	//}

	//fclose(infile);
	//fclose(outfile);
	close(sockfd);

	return 0;
}
