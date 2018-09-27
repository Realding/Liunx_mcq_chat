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


//connection fd
typedef struct client_fd{
	int fd;
	int id;
	char nickname[20];
	struct client_fd *next;
}client_fd;

//list Node
typedef struct{
	client_fd *head;
	client_fd *tail;
	int length;
}client_list;
/*client list*/
client_list clist;

void add_client(client_list* list,client_fd* cfd)
{
	printf("add \n");
	if(list -> length == 0)
	{
		list -> head = cfd;
		printf("head set\n");
		list -> tail = cfd;
		cfd -> next = NULL;
		list -> length++;
		
	}else
	{
		list -> tail -> next = cfd;
		list -> tail = cfd;
		cfd -> next = NULL;
		list -> length++;
		printf("change next\n");
	}
	
	

}
void init_client_list(client_list* list)
{
	list -> head = NULL;
	list -> tail = NULL;
	list -> length = 0;
	printf("init");
}
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
		printf("remove first\n");
		return p;
	}
	for(p=list->head;p->next!=NULL;p=p->next)
	{
		q = p->next;
		if(q -> id == id)
		{
			p->next = q->next;
			list->length--;
			printf("remove\n");
			return q;
		}
	}
	return NULL;
}

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

void *myThread1(void *arg)
{
	int nbytes;
    	char buffer[1024];
	client_fd client= *(client_fd *)arg;
	int accept_fd = client.fd;
	char memoPath[]="memo.txt";
	while(SocketConnected(accept_fd))
	{
		
		if((nbytes=read(accept_fd,buffer,1024))==-1)
		{
			fprintf(stderr,"Read Error:%s\n",strerror(errno));
			exit(1);
		}
		buffer[nbytes]='\0';
		
		FILE *m;
		
		/*receive quit cmd*/
		if(strcmp(buffer,"Q")==0||strcmp(buffer,"q")==0)
		{
			char sbuffer[100];
			sprintf(sbuffer,"[Notice:%s off-line]\n",client.nickname);
			
			printf("%s",sbuffer);
			
			if((m = fopen(memoPath,"a")) == NULL)
			{
				fprintf(stderr,"Memo Error:%s\n",strerror(errno));
			}else
			{
				fprintf(m,"%s",sbuffer);
				fclose(m);
			}

			client_fd *p;
			for(p=clist.head;p!=NULL;p=p->next)
			{
				if(p->id!=client.id)
					write(p->fd,sbuffer,strlen(sbuffer)); 
			}

			break;
		}

		printf("From user %s received: %s \n",client.nickname,buffer);

		//write to all
		char sbuffer[1024];
		time_t now;
		time(&now);
		char timeBuf[30];
		strftime(timeBuf,30,"%H:%M:%S", localtime(&now));
		sprintf(sbuffer,"[%s %s] said:\n%s\n",timeBuf,client.nickname,buffer);
		
		if((m = fopen(memoPath,"a")) == NULL)
		{
			fprintf(stderr,"Memo Error:%s\n",strerror(errno));
		}else
		{
			fprintf(m,"%s",sbuffer);
			fclose(m);
		}
		client_fd *p;
		printf("clist length:%d head id %d\n",clist.length,clist.head->id);
		for(p=clist.head;p!=NULL;p=p->next)
		{
			printf("Send to user %d\n",p->id);
			if(p->id!=client.id)
				write(p->fd,sbuffer,strlen(sbuffer)); 
		}

	}
	close(accept_fd);
	printf("connetion closed.\n");
	free(remove_client(&clist,client.id));
	printf("user %d remove\n",client.id);
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
	printf("MCQ server initializing...\n");



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
	printf("Tcp_server OK.listening...\n");

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
			printf("client connections full load.");
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
		printf("New user connected.fd:%d id:%d nickname:%s\n",client->fd,client->id,client->nickname);


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
	printf("server closed.\n");
	close(listen_fd);
	return 0;
}
		

