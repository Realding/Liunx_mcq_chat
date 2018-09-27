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


/*****************************************************/
/*	name : SocketConnected()
/*	function : if socket still connected
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
/*	name : myThread1()
/*	function : read from server
/*
/*****************************************************/
void *myThread1(void *arg)
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
		printf("%s\n",buffer);
	}
	close(accept_fd);
	printf("connetion closed.\n");
	return (void *) 0;
}

/*****************************************************/
/*	name : main()
/*	function :
/*
/*****************************************************/
int main(int argc, char *argv[]) 
{ 
	/*********************** init *******************************/
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
	/*********************** read *******************************/
	int n;
	pthread_t pt;	
	n==pthread_create(&pt,NULL,(void *)myThread1,(void *)&sockfd);     	
	if(n==0)
	{
		//printf("new thread created.\n");
		pthread_detach(pt);
	}
	/*********************** write *******************************/
	printf("\
===============================================================\n\
|      Welcome to mcq chat.Input 'Q' to quit chat.            |\n\
|      Please input your message and enjoy.                   |\n\
===============================================================\n");

	time_t now;
	//struct tm *timenow;
	strcpy(buffer,nickname);
	write(sockfd,buffer,strlen(buffer)); 

	while(strcmp(buffer,"Q")&&strcmp(buffer,"q")){
		/*Time*/
		time(&now);
		//timenow = localtime(&now);
		char timeBuf[BUFLEN];
 		strftime(timeBuf, BUFLEN, "%y/%m/%d %H:%M:%S", localtime(&now));
		printf("[ %s | Total_user:%c | %s]\n",timeBuf,'x',nickname);

		fgets(buffer,1024,stdin); 
		if(buffer[strlen(buffer)-1]=='\n')
			buffer[strlen(buffer)-1]='\0';
		write(sockfd,buffer,strlen(buffer)); 
	}
	close(sockfd); 
	exit(0); 
} 


