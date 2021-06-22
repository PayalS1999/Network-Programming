#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include "unp.h"
#include "chat_grp.h"

int main(int argc, char **argv)
{

  int sockfd;
  socklen_t servAddrLen;
  struct sockaddr_in cliAddr, servAddr;
  struct message msg1;

  char buffer[MAX_SIZE],grp[30];

  int i, j, n;

  pid_t p1;
  
  if(argc < 4) {
    printf("Usage : <myclient> <server IP address> <server port> <your user name> \n");
    exit(0);
  }

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0)
  {
    switch(errno)
    {
      case EACCES:
        printf("Permission to create socket of the specified type is denied\n");
      break; 

      case EAFNOSUPPORT:
        printf("Socket of given address family not supported\n");
      break;

      case EINVAL:
        printf("Invalid values in type\n");
      break;

      default:
        printf("Other socket errors\n");
      break;
    }
    exit(0);
  }

  bzero(&servAddr, sizeof(servAddr));
  bzero(&msg1,sizeof(msg1));
  servAddr.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &servAddr.sin_addr);
  servAddr.sin_port = htons(atoi(argv[2]));

  printf("TYPE 'join' TO START THE CHATTING SERVICE THEN PRESS [ENTER]");
  printf("\n\nEnter group number to which you want to connect \n Press 0 to start broadcasting or send to indivisual clien separately\n Enter your choice: ");
printf("\n\nAfter joining a group to send a message , type the message and press [ENTER]. \nThen enter the group number you want to sent message to or the particular user name pf the reciever to send a personnel message and press [ENTER]\n\n");

  p1=fork();
  if(p1==0)
  {
    while(1)
    {
	
	char *line=NULL;
	size_t linelen=0;
	int N;
	scanf("%d",&N);

	n= getline(&line, &linelen, stdin);
	scanf("%s",grp);

	strcpy(msg1.msg,line);

	if(strcmp(line,"join\n")==0)
	{

	    msg1.grp= atoi(&grp[0]);
	    strcpy(msg1.user,argv[3]);
	    
	}

	else if(strcmp(line,"leave\n")==0)
	{
	    msg1.grp= atoi(&grp[0]);

	    sendto(sockfd, &msg1, sizeof(msg1),0, (struct sockaddr *)&servAddr, sizeof(servAddr));
	    
	}
 
	else
	{

	    if(!isdigit(grp[0]))
	    {
		msg1.grp=10;
		strcpy(msg1.user,grp);
	    }

	    else
		{
	    	  msg1.grp= atoi(&grp[0]);
		  strcpy(msg1.user,argv[3]);
		}
	}
  	//printf("%lu\n", sizeof(msg1));
	free(line);
  	if(n>0)
    	  sendto(sockfd, &msg1, sizeof(msg1),0, (struct sockaddr *)&servAddr, sizeof(servAddr));
    }
  }

  if(p1>0)
  {
  	while(1)
  	{
  	n= recvfrom(sockfd, &buffer, sizeof(buffer), 0, (SA *)&servAddr, &servAddrLen);
  	if(n>0)
    	  fwrite(buffer,n,1,stdout);
    	  printf("\n\n");
  	fflush(stdout);
  }
  }
  
}
