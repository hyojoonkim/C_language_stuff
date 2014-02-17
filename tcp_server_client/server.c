/***********************
 * TCP Server code in C
 ***********************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define PORT_NUM	55001
#define BUF_SIZE	1024

enum {
	REQ_AUTH=1,
	RESP_AUTH
};

struct app_hdr 
{
	int   type;
	char  hwaddr[17];
	int   auth;
};

/* Send to controller */
int send_to_ctrl(int port_num)
{
	int sockfd;
	int bytes = 0;
	struct sockaddr_in server_addr;
	char snd_buf[BUF_SIZE];
	char rcv_buf[BUF_SIZE];

	struct app_hdr *snd_data;

	memset(snd_buf, 0, BUF_SIZE);
	memset(rcv_buf, 0, BUF_SIZE);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_num);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bzero(&(server_addr.sin_zero),8); 

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) 
	{
		perror("Connect to controller");
		exit(1);
	}

	snd_data = (struct app_hdr*) &snd_buf;
	snd_data->type = RESP_AUTH;

	/* send the message line to the controller */
	bytes = write(sockfd, snd_buf, BUF_SIZE);
	if (bytes < 0)  {
		perror("controller connect: Writing to socket");
		exit(1);
	}
	
	close(sockfd);

	return 0;
}

/* handle received data from client */
int handle_data(int connected, void *buf_ptr) 
{
	struct app_hdr *recv_data;
	struct app_hdr *send_data = NULL;

	recv_data = (struct app_hdr*) buf_ptr;

	printf("HA ANDLE\n");
	switch(recv_data->type) 
	{
		case REQ_AUTH:
			printf("AUTH !!! \n");
			send_data = malloc(sizeof(struct app_hdr));
			memset(send_data,0,sizeof(struct app_hdr));

			send_data->type = RESP_AUTH;
			memcpy(send_data->hwaddr,recv_data->hwaddr,6);
			send_data->auth = 1;

			send(connected, send_data, sizeof(struct app_hdr), 0);

			break;

		default:
			printf("type=%d\n",recv_data->type);
			break;
	}

	if(send_data)
		free(send_data);

	return 0;
}

/* main */
int main(int argc, char* argv[])
{
	int sockfd, connected, bytes;
    int optval = 1;
	char rcv_buf[BUF_SIZE];       

	struct sockaddr_in server_addr,client_addr;    
	uint32_t sin_size;

	// Create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket creation error");
		exit(1);
	}

	// Set socket option
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR, &optval, sizeof(int)) == -1) {
		perror("Setsockopt setting error");
		exit(1);
	}

	server_addr.sin_family = AF_INET;         
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.s_addr = INADDR_ANY; 
	bzero(&(server_addr.sin_zero),8); 

	// Bind
	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))==-1) {
		perror("Bind error");
		exit(1);
	}

	// Listen
	if (listen(sockfd, 5) == -1) {
		perror("Listen error");
		exit(1);
	}

	printf("\nTCPServer Waiting for client on port %d\n", PORT_NUM);
	fflush(stdout);

	// Polling..
	while(1)
	{  
		sin_size = sizeof(struct sockaddr_in);
		connected = accept(sockfd, (struct sockaddr *)&client_addr,&sin_size);
	
		// Recieve
		bytes = recv(connected, rcv_buf, BUF_SIZE,0);

		if(bytes) {
			handle_data(connected, &rcv_buf);
		}
		
		// connection close request
		else if(bytes==0) {
			close(connected);
			break;
		}

		else {
			perror("Recv error\n");
			close(connected);
			break;
		}

	}       

	close(sockfd);
	return 0;
} 

