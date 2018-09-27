#include <stdlib.h> 
#include <stdio.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <unistd.h>
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <pthread.h>
#include <netinet/tcp.h>
#define portnumber 8765
#define maxclient 100
#define MAXLINE 1024

//群聊客户端描述符
typedef struct client_fd{
	int fd;
	int id;
	char nickname[20];
	struct client_fd *next;
}client_fd;

//群聊列表
typedef struct{
	client_fd *head;
	client_fd *tail;
	int length;
}client_list;
/*client list*/
client_list clist;

//初始化客户端列表
void init_client_list(client_list* list)
{
	list -> head = NULL;
	list -> tail = NULL;
	list -> length = 0;
	printf("客户端列表初始化.\n");
}
//添加客户端
void add_client(client_list* list,client_fd* cfd)
{
	//printf("add \n");
	if(list -> length == 0)
	{
		list -> head = cfd;
		//printf("head set\n");
		list -> tail = cfd;
		cfd -> next = NULL;
		list -> length++;
		
	}else
	{
		list -> tail -> next = cfd;
		list -> tail = cfd;
		cfd -> next = NULL;
		list -> length++;
		//printf("change next\n");
	}
	
	

}
//移除客户端
client_fd* remove_client(client_list* list,int id)
{
	
	client_fd* p;
	client_fd* q;
	p=list->head;
	if(p->id == id)
	{
		list->head = p->next;
		if(list->length == 1)
			list->tail = NULL;
		list->length--;
		//printf("remove first\n");
		return p;
	}
	for(p=list->head;p->next!=NULL;p=p->next)
	{
		q = p->next;
		if(q -> id == id)
		{
			p->next = q->next;
			list->length--;
			//printf("remove\n");
			return q;
		}
	}
	return NULL;
}

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
/*	功能 : 处理与参加群聊的客户端的连接线程
/*
/*****************************************************/
void *myThread1(void *arg)
{
	int nbytes;
    	char buffer[1024];
	client_fd client= *(client_fd *)arg;
	int accept_fd = client.fd;
	char memoPath[]="memo.txt";

	int sl = 0;//私聊控制
	client_fd *sl_c;
	//登录通知
	client_fd *p;
	sprintf(buffer,"「系统通知」： %s 进入了聊天室\n",client.nickname);
	for(p=clist.head;p!=NULL;p=p->next)
	{
		if(p->id!=client.id)
		{
			write(p->fd,buffer,strlen(buffer)); 
		}
	}


	while(SocketConnected(accept_fd))
	{
		
		if((nbytes=read(accept_fd,buffer,1024))==-1)
		{
			fprintf(stderr,"Read Error:%s\n",strerror(errno));
			exit(1);
		}
		buffer[nbytes]='\0';
		
		FILE *m;
		printf("从用户「%s」接受到信息:\n  %s \n",client.nickname,buffer);
		/*判断用户是否退出*/
		if(strcmp(buffer,"Q")==0||strcmp(buffer,"q")==0)
		{
			char sbuffer[100];
			sprintf(sbuffer,"「系统通知」： %s 退出了聊天室\n",client.nickname);
			
			printf("%s",sbuffer);
			//聊天记录
			if((m = fopen(memoPath,"a")) == NULL)
			{
				fprintf(stderr,"Memo Error:%s\n",strerror(errno));
			}else
			{
				fprintf(m,"%s",sbuffer);
				fclose(m);
			}
			//发送通知给所有用户
			client_fd *p;
			for(p=clist.head;p!=NULL;p=p->next)
			{
				if(p->id!=client.id)
					write(p->fd,sbuffer,strlen(sbuffer)); 
			}
			break;
		}
		/*处理用户查询请求*/
		if((strcmp(buffer,"\\t")==0))
		{
			printf("向用户「%s」返回查询请求.\n",client.nickname);
			char sbuffer[1024];
			char tmpbuffer[1024];
			client_fd *p;
			time_t now;
			time(&now);
			char timeBuf[30];
			strftime(timeBuf,30,"%Y-%m-%d %H:%M:%S", localtime(&now));
			sprintf(sbuffer,"「系统提示」：当前在线人数：%d \t在线用户：",clist.length);
			for(p=clist.head;p!=NULL;p=p->next)
			{
				sprintf(tmpbuffer,"%s  |%s",sbuffer,p->nickname);
				strcpy(sbuffer,tmpbuffer);
			}
			write(accept_fd,sbuffer,strlen(sbuffer)); 
			continue;
			
		}
		/*处理文件传输请求*/
		if(buffer[0]=='\\'&&buffer[1]=='f')
		{
			char sbuffer[1024];
			if(sl)
			{
				sprintf(sbuffer,"f「系统提示」：用户%s请求传输文件%s，开始接收.",client.nickname,buffer+3);
				write(sl_c->fd,sbuffer,strlen(sbuffer));
				
				sprintf(sbuffer,"「系统提示」：文件传输请求成功，开始传输文件.");
				write(accept_fd,sbuffer,strlen(sbuffer));
				sleep(0.5);
				//sprintf(sbuffer,"agree");
				//write(accept_fd,sbuffer,strlen(sbuffer)); 
				int read_len,send_len;
				while ((read_len = read(accept_fd,buffer,1024)) >0 ) 
				{  
					send_len = send(sl_c->fd, buffer, read_len, 0); 
					printf("传输中..."); 
					if ( send_len < 0 ) 
					{  
						fprintf(stderr,"文件传输失败:%s\n",strerror(errno));
						exit(0);  
					}  
					if(strstr(buffer,"\\#")!=NULL)
						break;
					bzero(buffer, MAXLINE);  
				} 
				continue;

			}else
			{
				sprintf(sbuffer,"「系统提示」：请先向文件传输对象发起私聊，再使用文件传输命令.");
				write(accept_fd,sbuffer,strlen(sbuffer)); 
				
			}
			//sprintf(sbuffer,"donotagree");
			//write(accept_fd,sbuffer,strlen(sbuffer)); 
			continue;
		}
		/*私聊请求*/
		char ts[]="\\p";
		strncpy(ts,buffer,2);
		if((strcmp(ts,"\\s")==0))
		{
			char sbuffer[1024];
			if(sl)
			{
				sl = 0;
				sprintf(sbuffer,"「系统提示」：已退出与%s私聊",sl_c->nickname);
				write(accept_fd,sbuffer,strlen(sbuffer)); 
			}else
			{
				char tmpbuffer[100];//nickaname
				strcpy(tmpbuffer,buffer+3);
			
				client_fd *p;
				for(p=clist.head;p!=NULL;p=p->next)
				{
					if(strcmp(tmpbuffer,p->nickname)==0)
					{

						sl = 1;
						sl_c = p;
						sprintf(sbuffer,"「系统提示」：与%s私聊请求已建立，输入‘\\s’退出私聊模式",p->nickname);
						write(accept_fd,sbuffer,strlen(sbuffer)); 
						break;
					}
				}
				if(!sl)
				{
					sprintf(sbuffer,"「系统提示」：私聊请求失败，请检查私聊用户昵称输入是否正确");
					write(accept_fd,sbuffer,strlen(sbuffer)); 
				}
			}
			continue;
		}

		if(sl)
		{
			//私聊
			char sbuffer[1024];
			time_t now;
			time(&now);
			char timeBuf[30];
			strftime(timeBuf,30,"%H:%M:%S", localtime(&now));
			sprintf(sbuffer,"\033[31m\033[1m「私聊」[%s %s]:\033[0m\n\033[31m\033[1m  %s\033[0m",timeBuf,client.nickname,buffer);
			printf("%s发送私聊消息给用户:%s\n",client.nickname,sl_c->nickname);
			write(sl_c->fd,sbuffer,strlen(sbuffer)); 
			
		}else
		{
			//write to all
			char sbuffer[1024];
			time_t now;
			time(&now);
			char timeBuf[30];
			strftime(timeBuf,30,"%H:%M:%S", localtime(&now));
			sprintf(sbuffer,"\033[35m\033[1m「公聊」[%s %s]:\033[0m\n\033[34m\033[1m  %s\033[0m",timeBuf,client.nickname,buffer);
		
			if((m = fopen(memoPath,"a")) == NULL)
			{
				fprintf(stderr,"Memo Error:%s\n",strerror(errno));
			}else
			{
				fprintf(m,"%s",sbuffer);
				fclose(m);
			}
			client_fd *p;
			//printf("clist length:%d head id %d\n",clist.length,clist.head->id);
			for(p=clist.head;p!=NULL;p=p->next)
			{
				if(p->id!=client.id)
				{
					printf("发送消息给用户:%s\n",p->nickname);
					write(p->fd,sbuffer,strlen(sbuffer)); 
				}
			}
		}

	}
	close(accept_fd);
	printf("连接关闭.\n");
	free(remove_client(&clist,client.id));
	//printf("user %d remove\n",client.id);
	return (void *) 0;
}



/****************************************************************/
/*	name : main()
/*	function :
/*
/****************************************************************/


int main(int argc, char *argv[]) 
{ 


	/*********************** init *******************************/
	int listen_fd; 
	struct sockaddr_in client_addr; 
	printf("MCQ 服务器启动中……\n");



	/*********************** tcp *******************************/
	if((listen_fd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:IPV4;SOCK_STREAM:TCP
	{ 
		printf("Socket error:%s\n\a",strerror(errno)); 
		exit(1); 
	} 
	bzero(&client_addr,sizeof(struct sockaddr_in));
	client_addr.sin_family=AF_INET;                 // Internet
	client_addr.sin_addr.s_addr=htonl(INADDR_ANY);  // 
	//server_addr.sin_addr.s_addr=inet_addr("192.168.1.1"); 
	
	int n=1;
	setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&n,sizeof(int));  /* Enable address reuse */
        client_addr.sin_port=htons(portnumber);
	if(bind(listen_fd,(struct sockaddr *)(&client_addr),sizeof(client_addr))==-1) 
	{ 
		printf("Bind error:%s\n\a",strerror(errno)); 
		exit(1); 
	} 






	/*********************** listen *******************************/
	listen(listen_fd,5); 
	printf("服务器准备完毕.开始监听...\n");

	/*client list init*/

	init_client_list(&clist);
	int id_line = 10;	//id number
	while(1) 
	{
		/*accpet fd*/
		int accept_fd =accept(listen_fd,NULL,NULL);
		if(accept_fd<0)
                {
                      printf("Accept Error:%s\n",strerror(errno)); 
		      continue;
		} 

		/*new client*/

		if(clist.length >= maxclient)
		{
			printf("服务器达到最大连接数.\n");
			break;
		}
		client_fd *client = (client_fd *)malloc(sizeof(client_fd));
		client->fd = accept_fd;
		client->id = id_line++;
		client->next = NULL;
		//client = {accept_fd,id_line++,"",NULL};
		
		add_client(&clist,client);

		/*receive nick name*/

		char buffer[1024];int nbytes;
		if((nbytes=read(accept_fd,buffer,1024))==-1)
		{
			fprintf(stderr,"Read Error:%s\n",strerror(errno));
			continue;
		}
		buffer[nbytes]='\0';
		strcpy(client->nickname,buffer);
		printf("新用户已连接，fd:%d id:%d 昵称:%s\n",client->fd,client->id,client->nickname);


		/*new thread*/
		int n;
		pthread_t pt;	
		n==pthread_create(&pt,NULL,(void *)myThread1,(void *)client);
		if(n==0)
		{
			pthread_detach(pt);
		}
		else
		{
		        close(accept_fd);
		}

	} 
	printf("服务器关闭.\n");
	close(listen_fd);
	return 0;
}
		

