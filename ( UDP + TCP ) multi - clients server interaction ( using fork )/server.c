#include <stdio.h>
#include <unistd.h>
#include "unp.h"

int num_conn = 0;

void send_file_tcp(int, struct sockaddr_in , socklen_t);
void send_file_udp(struct message, int , char *, int);
void udp_reply(int, char *rxMsg,struct sockaddr_in cliAddr, socklen_t clilen);
int check_size(char *);
void create_child(char *, char *);

int main(int argc, char **argv)
{
  int listenfd, connfd, udpfd, i,n;
  struct sockaddr_in cliAddr, servAddr;
  socklen_t clilen;
  char rxMsg[MAX_SIZE];
  pid_t tcp_ch,conn_ch;

  udpfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

  bzero(&servAddr, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(atoi(argv[1]));

  if(bind(udpfd, (SA *)&servAddr, sizeof(servAddr)) < 0)
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
  clilen = sizeof(cliAddr);

  if(tcp_ch=fork()==0)
  {
    listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(bind(listenfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
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

    if(listen(listenfd, LISTENQ) < 0)
    {
      printf("Listen error ...\n");
      exit(0);
    }
    while(1)
    {
	connfd = accept(listenfd, (SA *)&cliAddr, &clilen);
	if(connfd>0)
	{
	  if(conn_ch= fork() == 0)
	  {
	    printf("listenfd = %d, connfd =  %d\n", listenfd, connfd);
            close(listenfd);
	    send_file_tcp(connfd,cliAddr,clilen);
	    exit(0);
	  }
          else
         {
           printf("Child pid = %d\n", conn_ch);
           waitpid(conn_ch, NULL, 0);
         }
	    close(connfd);
	}
    }

  }

  else
  {
     while(1)
     {
	n=recvfrom(udpfd, &rxMsg, sizeof(rxMsg), MSG_DONTWAIT, (SA *)&cliAddr, &clilen);
	if(n>0)
	{
	printf("UDP Message");
	udp_reply(udpfd,rxMsg,cliAddr,clilen);
	}
     }
     wait(NULL);
  }
}

void udp_reply(int udpfd, char *rxMsg,struct sockaddr_in cliAddr, socklen_t clilen)
{
  char *ip,port[10];
  int p;
  struct message txMsg;

  ip= inet_ntoa(cliAddr.sin_addr);
  printf("%s:", ip);
  p= ntohs(cliAddr.sin_port);
  printf("%d\n", p);
  snprintf(port,sizeof(port),"%d",p);

  strcpy(txMsg.fname,ip);
  strcat(txMsg.fname,":");
  strcat(txMsg.fname,port);
  strcat(txMsg.fname,".txt");

  create_child(txMsg.fname,rxMsg);
	
  if(check_size(txMsg.fname)> threshold)
  {
    txMsg.flag=1;
    txMsg.port=p;
    sendto(udpfd, &txMsg, sizeof(txMsg), 0, (SA *)&cliAddr, clilen); 
  }
  else
    send_file_udp(txMsg,udpfd,ip,p);
}

void create_child(char *fname, char *cmd)
{
  pid_t childpid;
  char *args[] = {"./command_exec.sh", NULL, NULL,NULL};
  char *token;
  
  args[1] = (char *)malloc(30*sizeof(char));
  memset(args[1], 0x0, 30);

  args[2] = (char *)malloc(30*sizeof(char));
  memset(args[1], 0x0, 30);

  strcpy(args[1],fname);
  strcpy(args[2],cmd);

  if((childpid = fork()) == 0)
  {
    if(execvp("./command_exec.sh", args) == -1)
            fprintf(stderr, "Unable to execute bash program\n");
    exit(0);
  }
  else{
    wait(NULL);
  }
}

int check_size(char *fname)
{
  FILE *fp;
  int size;

  fp= fopen(fname,"r");
  fseek(fp,0,SEEK_END);
  size=  ftell(fp);

  return size;
}

void send_file_udp(struct message txMsg,int sockfd, char *ip, int port)
{
  FILE *fp;

  struct sockaddr_in cliAddr; 
  socklen_t clilen;

  int n;

  bzero(&cliAddr, sizeof(cliAddr));
  cliAddr.sin_family = AF_INET;
  clilen = sizeof(cliAddr);

  inet_pton(AF_INET, ip, &cliAddr.sin_addr);
  cliAddr.sin_port = htons(port);

  txMsg.flag=0;
  fp= fopen(txMsg.fname,"r");

   while(1)
   {
     	n= fread(&txMsg.msg, 1, MAX_SIZE, fp);
        if (n < MAX_SIZE)
	{
	  printf("File transfer complete\n");
	  sendto(sockfd, &txMsg, sizeof(txMsg),0, (SA *)&cliAddr, clilen);
	  fclose(fp);
	  break;
	}
        sendto(sockfd, &txMsg, sizeof(txMsg),0, (SA *)&cliAddr, clilen);
   }

}

void send_file_tcp(int connfd, struct sockaddr_in cliAddr, socklen_t clilen)
{
  FILE *fp;
  char txMsg[MAX_SIZE], *ip,port[10];
  int n,p;

  ip= inet_ntoa(cliAddr.sin_addr);
      //printf("%s:", ip);
  p= ntohs(cliAddr.sin_port);
    	//printf("%d\n", p);
  snprintf(port,sizeof(port),"%d",p);

  strcpy(txMsg,ip);
  strcat(txMsg,":");
  strcat(txMsg,port);
  strcat(txMsg,".txt");

  fp= fopen(txMsg,"r");

  while(1)
  {
     n= fread(txMsg, 1, MAX_SIZE, fp);
     if (n < MAX_SIZE)
     {
	send(connfd, txMsg, n, 0);
	printf("File transfered on TCP connection\n");
	fclose(fp);
	break;
     }
     send(connfd, txMsg, n, 0);;
  }
}
