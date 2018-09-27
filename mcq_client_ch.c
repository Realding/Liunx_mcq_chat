#include <stdlib.h> 
#include <stdio.h> 
#include <errno.h> 
#include <unistd.h>
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <pthread.h>
#include <netinet/tcp.h>
#include <time.h>
#define portnumber 8765
#define BUFLEN 30
#define MAXLINE 1024


/*****************************************************/
/*	函数名 : SocketConnected()
/*	功能 : 判断连接是否断开
/*
/*****************************************************/
int SocketConnected(int sock) 
{ 
	if(sock<=0) 
		return 0; 
	struct tcp_info info; 
	int len=sizeof(info); 
	getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len); 
	if((info.tcpi_state==TCP_ESTABLISHED)) 
	{ 
		//myprintf("socket connected\n"); 
		return 1; 
	} 
	else 
	{ 
		//myprintf("socket disconnected\n"); 
		return 0; 
	} 
}
/*****************************************************/
/*	函数名 : myThreadRead()
/*	功能 : 从服务端读取信息
/*
/*****************************************************/
void *myThreadRead(void *arg)
{
	int nbytes;
    	char buffer[1024];
	int accept_fd = *(int *)arg;
	while(SocketConnected(accept_fd))
	{
		if((nbytes=read(accept_fd,buffer,1024))==-1)
		{
			fprintf(stderr,"Read Error:%s\n",strerror(errno));
			exit(1);
		}
		buffer[nbytes]='\0';
		if(buffer[0]=='f')
		{
			char answer[4];
			printf("%s\n",buffer+1);
			write(accept_fd,answer,strlen(answer)); 
			FILE *fp;
			printf("准备保存文件,文件存储为 ‘a’ .\n");
			char filePath[1024];
			strcpy(filePath,"a");
			if((fp = fopen(filePath,"w")) == NULL)
			{
				fprintf(stderr,"文件名错误，无法创建文件:%s\n",strerror(errno));
			}else
			{
				printf("开始接受.\n");
				bzero(buffer, MAXLINE); 
				int recv_len;
				int write_leng;
				while (recv_len = recv(accept_fd, buffer, MAXLINE, 0)) 
				{  
					if(recv_len < 0) 
					{  
						printf("文件接收失败!\n");  
						break;  
					}  
					printf("#");
					//printf("%s\n",buffer); 
					if(strstr(buffer,"\\#")!=NULL)
					{
						buffer[strlen(buffer)-2]=buffer[strlen(buffer)];//删去结束符/#
						write_leng = fwrite(buffer, sizeof(char), recv_len-2, fp);  
						if (write_leng < recv_len-2) 
						{  
							printf("文件写入失败！\n");  
							break;  
						}  
						break; 
					}
					write_leng = fwrite(buffer, sizeof(char), recv_len, fp);  
					if (write_leng < recv_len) 
					{  
						printf("文件写入失败！\n");  
						break;  
					}  
					
					bzero(buffer,MAXLINE);  
				}  
				fclose(fp);
				printf("\n文件传输完成！\n");  
			}
			continue;
			
		}
		printf("%s\n",buffer);//清除控制
		
	}
	close(accept_fd);
	printf("连接关闭.\n");
	return (void *) 0;
}

/*****************************************************/
/*	函数名 : main()
/*	功能 : 主函数
/*
/*****************************************************/
int main(int argc, char *argv[]) 
{ 
	/*********************** 初始化 *******************************/
	file_Send_agree=0;
	int sockfd; 
	char buffer[1024];
	char nickname[20];
	struct sockaddr_in server_addr; 
	struct hostent *host; 

	if(argc!=3) 
	{ 
		fprintf(stderr,"Usage:%s hostname nickname\a\n",argv[0]); 
		exit(1); 
	} 

	if((host=gethostbyname(argv[1]))==NULL) 
	{ 
		fprintf(stderr,"Gethostname error\n"); 
		exit(1); 
	} 
	strcpy(nickname,argv[2]);
	
	/*********************** tcp *******************************/
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:Internet;SOCK_STREAM:TCP
	{ 
		fprintf(stderr,"Socket Error:%s\a\n",strerror(errno)); 
		exit(1); 
	} 
 
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family=AF_INET;          // IPV4
	server_addr.sin_port=htons(portnumber);  
	server_addr.sin_addr=*((struct in_addr *)host->h_addr); 
	
	
	if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) 
	{ 
		fprintf(stderr,"Connect Error:%s\a\n",strerror(errno)); 
		exit(1); 
	} 

	/*********************** 读线程 *******************************/
	int n;
	pthread_t pt;	
	n==pthread_create(&pt,NULL,(void *)myThreadRead,(void *)&sockfd);     	
	if(n==0)
	{
		//printf("new thread created.\n");
		pthread_detach(pt);
	}
	/*********************** 写信息至服务端 *******************************/
	
	printf("===============================================\n\
客户端已登录,您已进入聊天室，输入‘Q’或‘q’退出;\n输入‘\\t’查看看当前在线用户;\n\
输入「‘\\s 用户昵称’」 发送私聊请求;\n\
输入「‘\\f 文件路径’」向私聊对象发送文件.\n\
===============================================\n");	

	//发送给服务端描述信息
	strcpy(buffer,nickname);
	write(sockfd,buffer,strlen(buffer)); 

	time_t now;
	//struct tm *timenow;
	//发送聊天信息
	while(strcmp(buffer,"Q")&&strcmp(buffer,"q")){
		//请求查询用户
		if(strcmp(buffer,"\\t")==0)
		{
			printf("「发送查询请求」\n");
		}else if(buffer[0]=='\\'&&buffer[1]=='f')
		{
			printf("「请求发送文件」\n");
			char filePath[1024];
			strcpy(filePath,buffer+3);
			FILE *fp;

			if((fp = fopen(filePath,"r")) == NULL)
			{
				fprintf(stderr,"文件名错误，找不到文件:%s\n",strerror(errno));
			}else
			{
				printf("等待传输\n");
				sleep(0.2);
				int read_len,send_len;
				while ((read_len = fread(buffer, sizeof(char), MAXLINE, fp)) >0 ) 
				{  
					//printf("%s\n",buffer); 
					send_len = send(sockfd, buffer, read_len, 0);  
					if ( send_len < 0 ) 
					{  
						fprintf(stderr,"文件传输失败:%s\n",strerror(errno));
						exit(0);  
					}  
					bzero(buffer, MAXLINE);  
				} 
				printf("发送完毕.\n");
				fclose(fp);  
				sprintf(buffer,"\\#");
				send(sockfd, buffer, strlen(buffer), 0);  
			}
		}
		else		
		{
			printf("\033[33m\033[1m");//
			/*Time*/
			time(&now);
			//timenow = localtime(&now);
			char timeBuf[BUFLEN];
			strftime(timeBuf,30,"%H:%M:%S", localtime(&now));
		 	//strftime(timeBuf, BUFLEN, "%m月%d日 %H:%M:%S", localtime(&now));
			printf("「我」[%s %s]:\n",timeBuf,nickname);
			printf("\033[0m");//清除控制
		}
		printf("\033[32m\033[1m");//
		fgets(buffer,1024,stdin); 
		printf("\033[0m");//清除控制
		if(buffer[strlen(buffer)-1]=='\n')
			buffer[strlen(buffer)-1]='\0';
		write(sockfd,buffer,strlen(buffer)); 

	}
	close(sockfd); 
	exit(0); 
} 


