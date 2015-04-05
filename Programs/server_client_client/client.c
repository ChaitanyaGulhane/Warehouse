#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>

#define MAXLEN 1024
#define SERV_PORT 3000
 
int main(int argc, char **argv)
{
	if(argc < 4)
	{
		fprintf(stderr, "Arguement insufficient...!!!");
		return -1;
	}
	
	struct sockaddr_in server_sock;
	int sockfd;

        if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		fprintf(stderr, "socket() error => %s ", strerror(errno));
		return -1;
	}

        memset(&server_sock, 0, sizeof(server_sock));
        server_sock.sin_family = AF_INET;
        server_sock.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, argv[1], &server_sock.sin_addr);

        if(connect(sockfd, (struct sockaddr *)& server_sock, sizeof(server_sock)) == -1)
	{
		close(sockfd);
		return -1;
	}

	FILE * infile = fopen(argv[2], "r");
	FILE * outfile = fopen(argv[3], "w");
	
	char line[MAXLEN];
	int len ;

	while(fgets(line, MAXLEN , infile) )
	{
		if(write(sockfd, line, strlen(line)) < 0)
			break;

		if((len = read(sockfd, line, MAXLEN)) < 0)
			break;

		line[len] = 0;
		printf("%s", line);		

		fputs(line, outfile);

	}

	fclose(infile);
	fclose(outfile);
	close(sockfd);

	return 0;
}
