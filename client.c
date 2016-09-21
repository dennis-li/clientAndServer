#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

#define PROTOPORT 5188
extern int errno;
char *localhost = "localhost";
void thread(int *sockfd);

int main(int argc,char *argv[])
{
	struct hostent *ptrh;		//指向主机列表一个条目
	struct sockaddr_in servaddr;	//存放服务器网络地址结构
	int sockfd;			//客户机套接字描述符
	int port;			//服务器套接字端口号
	char *host,*msg;			//服务器主机名
	int n;				//读取的字符数
	char buf[1000];			//缓冲区，接受服务器发来的字符

	memset((char *)& servaddr,0,sizeof(servaddr));	//清空sockaddr结构
	servaddr.sin_family = AF_INET;  		//设置为internet协议族

	if(argc > 2)
		port = atoi(argv[2]);			//如果指定了协议端口，就转换位整数
	else
		port = PROTOPORT;			//否则，使用默认端口号
	
	//如果是合法的端口号，就装入网络地址结构
	if (port > 0)					
		servaddr.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"bad port number %s \n",argv[2]);
		exit(1);
	}

	//检查主机参数，并指定主机名
	if(argc > 1)
		host = argv[1];
	else
		host = localhost;
	
	//将主机名转换成相应的IP地址，并存到servaddr结构中
	ptrh = gethostbyname(host);			//通过主机名，获取对应的IP地址

	//检查主机名的有效性
	if((char *)ptrh == NULL) {
		fprintf(stderr,"invalid host: %s\n",host);
		exit(1);
	}

	memcpy(&servaddr.sin_addr,ptrh->h_addr,ptrh->h_length); //把IP地址，长度，存放到servaddr结构中

	//创建一个tcp/ip,流式套接字
	sockfd = socket(AF_INET, SOCK_STREAM,0);
	if(sockfd < 0){
		fprintf(stderr,"socket creation failed\n");
		exit(1);
	}

	//请求连接服务器
	if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0){
		fprintf(stderr,"connect faiiled\n");	//连接被拒绝
		exit(1);
	}
	
	pthread_t id;
	int ret = pthread_create(&id,NULL,(void *)thread,&sockfd);//创建线程
	if(ret != 0)
	{
		printf("create thread error\n");
		exit(1);
	}

	//向服务器发送消息
	while(scanf("%s",msg) != EOF)
	{
		write(sockfd,msg,strlen(msg));
	}
	pthread_join(id,NULL);

	//关闭套接字
	close(sockfd);

	//终止程序
	exit(0);
}

void thread(int *sockfd)
{
	int n;
	char buf[1000];
	n = recv(*sockfd,buf,sizeof(buf),0);
	while(n > 0)
	{
		write(1,buf,n);
		n = recv(*sockfd,buf,sizeof(buf),0);
	}
}
