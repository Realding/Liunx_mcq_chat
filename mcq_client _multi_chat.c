#include <stdlib.h> 
#include <stdio.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <pthread.h>
#include <netinet/tcp.h>
#define portnumber 5678

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
int main(int argc, char *argv[]) 
{ 
	int sockfd; 
	char buffer[1024]; 
	struct sockaddr_in server_addr; 
	struct hostent *host; 

	if(argc!=2) 
	{ 
		fprintf(stderr,"Usage:%s hostname \a\n",argv[0]); 
		exit(1); 
	} 

	if((host=gethostbyname(argv[1]))==NULL) 
	{ 
		fprintf(stderr,"Gethostname error\n"); 
		exit(1); 
	} 

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
	//read
	int n;
	pthread_t pt;	
	n==pthread_create(&pt,NULL,(void *)myThread1,(void *)&sockfd);     	
	if(n==0)
	{
		//printf("new thread created.\n");
		pthread_detach(pt);
	}
	//write
	printf("\
		*****************************************\n\
		|\tWelcome to mcq chat.      \t|\n\
		|\tInput 'Q' to quit chat.   \t|\n\
		|\tPlease input your message:\t|\n\
		-----------------------------------------\n");
	while(strcmp(buffer,"Q") != 0){
		
		fgets(buffer,1024,stdin); 
		if(buffer[strlen(buffer)-1]=='\n')
			buffer[strlen(buffer)-1]='\0';
		write(sockfd,buffer,strlen(buffer)); 
	}
	close(sockfd); 
	exit(0); 
} 


