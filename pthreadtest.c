#include <stdio.h> 
#include <unistd.h>
#include <pthread.h>

void myThread1(){
	printf("mythread start");
}

int main(int argc, char *argv[]) 
{

	int n;
	pthread_t pt;	
	n==pthread_create(&pt,NULL,(void *)myThread1,(void *));     	
	if(n==0)
	{
		//printf("new thread created.\n");
		pthread_detach(pt);
	}
	exit(0); 
} 

