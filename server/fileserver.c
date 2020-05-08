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
 
/* ȫ�ֱ����� */
char  filename[FILE_MAX_LEN+1];
 
/*  handle_cilent����  */
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

	/* ���ļ��� */
	ret = send(sock, filename, FILE_MAX_LEN, 0);
	if(-1 == ret)
	{
		perror("Send file name");
		goto EXIT_THREAD;
	}
 
	printf("Begin send file %s ...\n", filename);

	/* �����ļ����� */
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
	
	/* ����2��IPV4��ַ */
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
 
	/* ��һ��:����TCP�׽���socket */
	/* AF_INET :ipͨ��  SOCK_STREAM:TCP */
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sockfd)
	{
		perror("socket");
		exit(2);
	}
 
	/* �ڶ���:���ü����˿� ����:��ʼ���ṹ��,����2828�˿� */
	memset(&my_addr, 0, sizeof(struct sockaddr));
	my_addr.sin_family = AF_INET;  // IPV4 
	my_addr.sin_port = htons(port); //���ü����˿�Ϊ2828,��htonsת�������� 
//	my_addr.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY����ʾ����IP��ַ����ͨ��
	my_addr.sin_addr.s_addr = htons(INADDR_ANY); //INADDR_ANY����ʾ����IP��ַ����ͨ��
 
	/* ������:���׽���,��socket�����ڶ˿ڹ������� */
	ret = bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr));
	if(-1 == ret)
	{
		perror("bind error");
		goto EXIT_MAIN;
	}
 
	/* ���Ĳ�:��ʼ��2828�˿ڼ���,�Ƿ��пͻ��˷������� */
	ret = listen(sockfd, 10);
	if(-1 == ret)
	{
		perror("listen");
		goto EXIT_MAIN;
	}
	printf("Listen Port:%d !\n",port);
 
	/* ѭ����ͻ���ͨ�� */
	while(1)
	{
		sin_size = sizeof(struct sockaddr_in);
		printf("\nServer waiting ... \n");
 
		//����пͻ��˽�������,�������һ��ȫ�µ��׽���newfd,ר�����ں�����ͻ���ͨ�� 
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

		//����һ���߳��������ͻ��˵ĶԻ�,�����̼�������
		pthread_create(&thread, NULL, handle_cilent, (void *)newfd);
	}
 
	/* ������:�ر�Socket */
	EXIT_MAIN:
	close(sockfd);
	return 0;
}

