#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
//#define DEFAULT_SVR_PORT 6776
#define DEFAULT_SVR_PORT 8888
#define FILE_MAX_LEN 64
 
/* 全局变量区 */
char  filename[FILE_MAX_LEN+1];
 
/*  handle_cilent函数  */
static void *handle_cilent(void *arg)
{
	int sock = (int)arg;
	char buf[1024], buff[2048];
	int len,ret;
	FILE *fp, *fpp;
 
	printf("Begin Send!\n");
	fp = fopen(filename, "r");
	if(NULL == fp)
	{
		close(sock);
		exit(6);
	}

	/* 发文件名 */
	ret = send(sock, filename, FILE_MAX_LEN, 0);
	if(-1 == ret)
	{
		perror("Send file name");
		goto EXIT_THREAD;
	}
 
	printf("Begin send file %s ...\n", filename);

	/* 发送文件内容 */
	int cnt=0;
	while(!feof(fp))
	{
		len = fread(buf, 1, sizeof(buf), fp);
 
		printf("Server read %s, len %d\n", filename, len);
 
		ret = send(sock, buf, len, 0);
		if(ret < 0)
		{
			perror("Send file content");
			goto EXIT_THREAD;
		}
		memset(buf, 0, sizeof(buf));
	}

	EXIT_THREAD:
	if(fp)
	{
		fclose(fp);
	}
	close(sock);
}

int main(int argc , char *argv[])
{
	int sockfd,newfd=-1;
	int ret;
	int sin_size,numbytes;
	
	/* 定义2个IPV4地址 */
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	pthread_t thread;
	unsigned short port;
	
	if(argc < 2)
	{
		printf("Please input a filename without path\n");
		exit(1);
	}
 
	strncpy(filename, argv[1], FILE_MAX_LEN);
	port = DEFAULT_SVR_PORT;
	if(argc >= 3)
	{
		port = (unsigned short)atoi(argv[2]);
	}
 
	/* 第一步:建立TCP套接字socket */
	/* AF_INET :ip通信  SOCK_STREAM:TCP */
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sockfd)
	{
		perror("socket");
		exit(2);
	}
 
	/* 第二步:设置监听端口 功能:初始化结构体,并绑定2828端口 */
	memset(&my_addr, 0, sizeof(struct sockaddr));
	my_addr.sin_family = AF_INET;  // IPV4 
	my_addr.sin_port = htons(port); //设置监听端口为2828,用htons转成网络序 
//	my_addr.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY来表示任意IP地址可能通信
	my_addr.sin_addr.s_addr = htons(INADDR_ANY); //INADDR_ANY来表示任意IP地址可能通信
 
	/* 第三步:绑定套接字,把socket队列于端口关联起来 */
	ret = bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr));
	if(-1 == ret)
	{
		perror("bind error");
		goto EXIT_MAIN;
	}
 
	/* 第四步:开始在2828端口监听,是否有客户端发来连接 */
	ret = listen(sockfd, 10);
	if(-1 == ret)
	{
		perror("listen");
		goto EXIT_MAIN;
	}
	printf("Listen Port:%d !\n",port);
 
	/* 循环与客户端通信 */
	while(1)
	{
		sin_size = sizeof(struct sockaddr_in);
		printf("\nServer waiting ... \n");
 
		//如果有客户端建立连接,将会产生一个全新的套接字newfd,专门用于和这个客户端通信 
		newfd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
		if(-1 == newfd)
		{
			perror("accept");
			goto EXIT_MAIN;
		}

		char ipv4_addr[16];
		if(!inet_ntop(AF_INET, (void *)&their_addr.sin_addr, ipv4_addr, sizeof(their_addr)))
		{
			perror("inet_ntop");
			goto EXIT_MAIN;
		}
		printf("Client (ip = %s : port = %d ) request !\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port));

		//生成一个线程来完成与客户端的对话,父进程继续监听
		pthread_create(&thread, NULL, handle_cilent, (void *)newfd);
	}
 
	/* 第六步:关闭Socket */
	EXIT_MAIN:
	close(sockfd);
	return 0;
}

