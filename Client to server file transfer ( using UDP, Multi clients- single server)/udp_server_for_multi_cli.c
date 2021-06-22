#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "unp.h"

#define MAX_SIZE 500
int main(int argc, char *argv[])
{
  int sockfd;
  struct sockaddr_in servAddr, cliAddr; 
  socklen_t cliAddrLen;

  char rxMsg[MAX_SIZE],str[32],fname[100];
  char success[]="SUCCESS";
  int port;
  FILE *fp;

  int n;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  bzero(&servAddr, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(atoi(argv[1]));
  if(bind(sockfd, (SA *)&servAddr, sizeof(servAddr)) < 0)
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
  cliAddrLen = sizeof(cliAddr);

  while(1)
  {
    n= recvfrom(sockfd, &rxMsg, sizeof(rxMsg), 0, (SA *)&cliAddr, &cliAddrLen);
    
    strcpy(fname,"msg_");

    inet_ntop(AF_INET, &cliAddr.sin_addr, str, 32);
    strcat(fname,str);

    strcat(fname,"_");

    port= ntohs(cliAddr.sin_port);
    snprintf(str,sizeof(str),"%d",port);
    strcat(fname,str);

    fp= fopen(fname,"a+");
    fwrite(rxMsg,1,n,fp);

    fclose(fp);

    if(n < MAX_SIZE)
    	sendto(sockfd, &success, sizeof(success),0, (SA *)&cliAddr, cliAddrLen);
  }

}
