#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <pthread.h>
#include <limits.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SOCKETS_BUFFER_SIZE  1024
#define SOCKETS_TIMEOUT      2
#define FILE_MAX_LEN 64

void *network_connect(void *arg)
{
	int socketfd, port;
	char *ip_address;
	struct sockaddr_in MyAddress, MyControlAddress;
	int status;

	/* Create socket (allocate resources) - IPv4, TCP */
	socketfd = socket(AF_INET, SOCK_STREAM, 0);

	if (socketfd == -1) {
		printf("Error: Unable to create socket (%i)...\n", errno);
	}

	/* Establish TCP connection */
	port=8888;
	ip_address="192.168.0.104";
	memset(&MyAddress, 0, sizeof(struct sockaddr_in));
	MyAddress.sin_family = AF_INET;
	MyAddress.sin_port = htons(port);
	MyAddress.sin_addr.s_addr = inet_addr(ip_address);

printf("port=%d\n", port);
printf("ip_address=%s\n", ip_address);
	status = connect(socketfd, (struct sockaddr *)&MyAddress, sizeof(struct sockaddr_in));
	if(status == -1) {
		printf("Error: Unable to establish connection to ip:%s (%i)...\n", ip_address, errno);
	}

	int total, len;
	char filename[FILE_MAX_LEN+1], buf[1024];
	FILE *fp;

	/* 接收文件名 */
	total = 0;
	while (total < FILE_MAX_LEN)
	{
		len = recv(socketfd, filename+total, (FILE_MAX_LEN-total), 0);
		if(len == -1)
		{
			printf("recv filename fail\n");
			break;
		}
		total += len;
	}

	/* 接收文件名出错 */
	if(total != FILE_MAX_LEN)
	{
		perror("failure file name");
	}
	printf("recv file %s ...\n", filename);

	fp = fopen(filename, "w");
	if(NULL == fp)
	{
		perror("open");
	}

	/* 接收文件数据 */
	printf("\nRecv File Begin!\n");
	total = 0;
	while(1)
	{
		len = recv(socketfd, buf, sizeof(buf), 0);

		if(!len)
		{
			printf("recv data finished\n");
			break;
		}
		total += len;
//		printf("total=%d len=%d\n", total, len);
		//写入本地文件
		fwrite(buf, 1, len, fp);
	}

	fclose(fp);
	printf("Recv file %s success! Total length:%d  \n", filename, total);
}


int main(int argc, char **argv)
{
	pthread_t tid;

	pthread_create(&tid, NULL, network_connect, NULL);

	int ret=pthread_join(tid, NULL);
	if(!ret)
		printf("Thread joined\n");
	else
		printf("Thread join failed\n");

	return 0;
} 

