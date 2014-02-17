/***********************
 * TCP Client code is C
 ***********************/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define PORT_NUM	55001
#define BUF_SIZE	1024

enum {
	REQ_AUTH=1,
	RESP_AUTH
};

struct app_hdr 
{
	int type;
	char hwaddr[6];

	int auth;
	int d2;
	int d3;
};

/* main */
int main(int argc, char* argv[])
{
	int sockfd;
	int bytes = 0;
	//struct hostent *host, *c_host;
	struct sockaddr_in server_addr;
	char snd_buf[BUF_SIZE];
	char rcv_buf[BUF_SIZE];

	struct app_hdr *snd_data;

	//host = gethostbyname("128.61.3.4");
	memset(snd_buf, 0, BUF_SIZE);
	memset(rcv_buf, 0, BUF_SIZE);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bzero(&(server_addr.sin_zero),8); 

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) 
	{
		perror("Connect");
		exit(1);
	}

	snd_data = (struct app_hdr*) &snd_buf;
	snd_data->type = REQ_AUTH;

	/* send the message line to the server */
	//bytes = write(sockfd, snd_buf, strlen(snd_buf));
	bytes = write(sockfd, snd_buf, BUF_SIZE);
	if (bytes < 0)  {
		perror("Writing to socket");
		exit(1);
	}

	/* print the server's reply */
	bytes = read(sockfd, rcv_buf, BUF_SIZE);
	if (bytes < 0) {
		perror("Reading from socket");
		exit(1);
	}

	printf("Echo from server: %s", rcv_buf);
	close(sockfd);
	
	return 0;
}
