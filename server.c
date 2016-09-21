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
#include<unistd.h>

#define PROTOPORT 5188
#define QLEN 2

int visits = 0;
char buf[1000];
void thread(int *);

int main(int argc,char *argv[])
{
	        struct hostent *ptrh;			//指向主机列表一个条目
		struct sockaddr_in servaddr;		//存放服务器网络地址结构
		int listenfd,pid;				//监听套接字描述符
		int clientfds[1000];				//响应套接字描述符
		int port;				//协议端口号
		unsigned int alen;				//地址长度
		char buf[1000];				//服务器发送字符串的缓冲区
		char *msg;

		memset((char *)& servaddr,0,sizeof(servaddr));	//清空sockaddr结构体
		servaddr.sin_family = AF_INET;			//设置位internet协议簇
		servaddr.sin_addr.s_addr = INADDR_ANY;		//设置本地IP

		//若有指定端口号，就用指定端口号，没有就是用默认端口号
		if(argc > 1)
			port = atoi(argv[1]);
		else
			port = PROTOPORT;

		if(port > 0)
			servaddr.sin_port = htons((u_short)port);
		else {
		fprintf(stderr," bad port number %s\n",argv[1]);
			exit(1);
		}

		//创建一个用于监听的流式套接字
		listenfd = socket(AF_INET,SOCK_STREAM,0);
		if(listenfd < 0){
			fprintf(stderr,"socket creation failed\n");
			exit(1);
		}

		//将本地地址绑定到监听套接字
		if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
			fprintf(stderr, "bind failed\n");
			exit(1);
		}

		//开始监听，并指定监听套接字请求队列长度
		if(listen(listenfd,QLEN) < 0) {
			fprintf(stderr, "listen failed\n");
			exit(1);
		}

		//服务器循环处理客户端请求
		while(1) {
			int clientfd,i,n;
			struct sockaddr_in clientaddr;
			alen = sizeof(clientaddr); 		//接受客户端请求，并生成响应套接字
			if((clientfd = accept(listenfd,(struct sockaddr *)& clientaddr,&alen)) < 0){
				fprintf(stderr, "accept failed\n");
				exit(1);
			} else {
				clientfds[visits] = clientfd;
				visits++;				//累计访问客户机数
				for(i = 0;i < visits;i++){
					sprintf(buf,"你是第%d个用户 \n",visits);
					send(clientfds[i],buf,strlen(buf),0);	//向客户机端发送消息
				}

				pthread_t id;
				int ret = pthread_create(&id,NULL,(void *)thread,&clientfd);
        			if (ret != 0)
        			{
              				printf("Create thread error!\r\n");
              				exit(1);
         			}
         			pthread_join(id,NULL);
				
			}
			
			//close(clientfd);
		}
}

void thread(int *clientfd)
{
	int pid,n;
	char msg[1000];
	if((pid = fork()) ==  0)
	{
		while(1){
			while((n = read(*clientfd,msg,1000)) > 0)
			{
				msg[n] = '\0';
				printf("编号%d的消息:%s\n",*clientfd,msg);
			}
		}
	}
}
