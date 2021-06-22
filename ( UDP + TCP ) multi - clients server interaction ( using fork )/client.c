#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/select.h>
#include "unp.h"

socklen_t servAddrLen;
struct sockaddr_in cliAddr, servAddr;

void udp_conn(int,int);
void tcp_conn(int, int);

void tcp_conn(int,int);

int main(int argc, char **argv)
{
  int i, sockfd, tcpfd;
  char buffer[MAX_SIZE];
  pid_t ch;
  
  if(argc < 3) {
    printf("Usage : <myclient> <server IP address> <server port>\n");
    exit(0);
  }

  sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

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
  servAddr.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &servAddr.sin_addr);
  servAddr.sin_port = htons(atoi(argv[2]));

  if(ch=fork()==0)
  {
     while(1)
     {
       fgets(buffer,MAX_SIZE,stdin);
       printf("stdin selected\n");    
       printf("buffer read : %s",buffer);
       sendto(sockfd, &buffer, MAX_SIZE,0,(struct sockaddr *)&servAddr, sizeof(servAddr));
     }
  }
  else
  {
    sleep(6);
    while(1)
      udp_conn(sockfd,tcpfd);
  }
}

void udp_conn(int udpfd, int tcpfd)
{
   int n=0;
   char buffer[MAX_SIZE];
   struct message rxMsg;
   while(n<=0)
   n= recvfrom(udpfd, &rxMsg, sizeof(rxMsg), MSG_DONTWAIT, (SA *)&servAddr, &servAddrLen);

   printf("\n Recieved from socket");
   if(rxMsg.flag==1)
   {
  	tcpfd = socket(AF_INET, SOCK_STREAM, 0);
	printf("Tcp connection started\n");
	tcp_conn(tcpfd,rxMsg.port);		// to initiate TCP connection
	fflush(stdout);

	while(1)
    	{
	  n= recv(tcpfd, buffer, MAX_SIZE,0);
     	   if(n < MAX_SIZE)
      	   {
		fputs(buffer, stdout);
		fflush(stdout);
        	printf("completed recieving file on tcp\n");
        	break; 
      	   }

      	  fputs(buffer, stdout);
      	  bzero(buffer, MAX_SIZE);
    	}
	close(tcpfd);
   }
   else
   {
	while(1)
    	{
     	   if(n=recvfrom(udpfd, &rxMsg, sizeof(rxMsg), 0, (SA *)&servAddr, &servAddrLen)<MAX_SIZE)
	   {
	     fputs(rxMsg.msg, stdout);
	     fflush(stdout);
             printf("completed recieving file on udp\n");
             break; 
      	   }

      	     fputs(rxMsg.msg, stdout);
      	     //bzero(rxMsg, MAXLINE);
    	  }
       }
}

void tcp_conn(int tcpfd, int port)
{
  static int iter=1;

  if(iter==1)
  {
    bzero(&cliAddr, sizeof(cliAddr));
    cliAddr.sin_family = AF_INET;
    cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliAddr.sin_port = htons(port);
  }

    if(bind(tcpfd, (struct sockaddr *)&cliAddr, sizeof(cliAddr)) < 0)
    {
      if(errno == EADDRINUSE)
      {
        printf("Bind error ... Port is still in use\n");
      }
      else
      {
        printf("Bind error ... \n"); 
      }
      return;
    }
  iter++;

  if(connect(tcpfd, (SA *)&servAddr, sizeof(servAddr)) < 0)
  {
    switch(errno)
    {
      case ETIMEDOUT : 
        printf("Timed out for TCP SYN ..\n");
        break;

      case EHOSTUNREACH : 
        printf("Host unreachable error ...\n");
        break;

      case ENETUNREACH : 
        printf("Network unreachable error ...\n");
        break;

      case ECONNREFUSED : 
        printf("Connection refused on requested port ...\n");
        break;

      default :
        printf("Unknown connect error ...\n");
        break;
    }
    return;
  } 
}

