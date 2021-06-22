#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include "unp.h"

#define MAX_SIZE 500
int main(int argc, char **argv)
{

  int sockfd;
  socklen_t servAddrLen;
  struct sockaddr_in cliAddr, servAddr;

  FILE *fp;
  char buffer[MAX_SIZE];

  int i, j, n;
  
  if(argc < 3) {
    printf("Usage : <myclient> <server IP address> <server port>\n");
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
  servAddr.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &servAddr.sin_addr);
  servAddr.sin_port = htons(atoi(argv[2]));

  fp= fopen("serv_data.txt","r");

  if(fp==NULL)
  {
    printf("Could'nt open file");
    exit(0);
  }
  while(1)
  {
  n= fread(&buffer, 1, MAX_SIZE, fp);
  //printf("\n%d",n);
  sendto(sockfd, &buffer, n,0, (struct sockaddr *)&servAddr, sizeof(servAddr));
  if(n<MAX_SIZE && feof(fp)>0)
  {
	fclose(fp);
	break;
  }
  }

  if(n=recvfrom(sockfd, &buffer, sizeof(buffer), 0, (SA *)&servAddr, &servAddrLen))
    fwrite(buffer,1,n,stdout);
  
}
