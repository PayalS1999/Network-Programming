#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/select.h>
#include "unp.h"

int to_exit = 0, i, select_no,j;
socklen_t servAddrLen;
struct sockaddr_in cliAddr, servAddr;

void tcp_conn(int, int);

int main(int argc, char **argv)
{
  char buffer[MAX_SIZE];
  struct message rxMsg;
  int i, n, sockfd, tcpfd;
  fd_set readfd;
  
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

  while(1)
  { 
    j++;
    printf("\n %dth iteration ..\n", j);

    FD_ZERO(&readfd);
    FD_SET(fileno(stdin),&readfd);
    FD_SET(sockfd,&readfd);
    select_no=select(sockfd+1,&readfd,NULL,NULL,NULL);

    if(FD_ISSET(fileno(stdin),&readfd))				// Incoming in stdin
    {
      printf("stdin selected\n");    
      fgets(buffer,MAX_SIZE,stdin);
      printf("buffer read : %s",buffer);
      sendto(sockfd, &buffer, MAX_SIZE,0,(struct sockaddr *)&servAddr, sizeof(servAddr));
    }

    if(FD_ISSET(sockfd,&readfd))				// Incoming in UDP Socket
    {
       printf("socket selected\n");
       n= recvfrom(sockfd, &rxMsg, sizeof(rxMsg), MSG_DONTWAIT, (SA *)&servAddr, &servAddrLen);

       if(rxMsg.flag==1)					// Recieve on TCP socket
       {
	tcpfd = socket(AF_INET, SOCK_STREAM, 0);
	printf("Tcp connection started\n");
	tcp_conn(tcpfd,rxMsg.port);				// TCP connection initialisation
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
	close(tcpfd);						// Close TCP connection
       }
       else							// Recieve on UDP socket
       {
	  while(1)
    	  {
     	 	if(n=recvfrom(sockfd, &rxMsg, sizeof(rxMsg), 0, (SA *)&servAddr, &servAddrLen)<MAX_SIZE)
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
      exit(0);
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

      case EAI_SOCKTYPE:
	printf("1");
	break;

      case EBADF:
	printf("2");
	break;

      case EISCONN:
	printf("escon");
	break;

      default :
        printf("Unknown connect error ...\n");
        break;
    }
    exit(0);
  } 
}
