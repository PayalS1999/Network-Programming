#include <stdio.h>
#include <unistd.h>
#include "unp.h"

int num_conn = 0;

void send_file_tcp(int, char *);
void send_file_udp(struct message, int , char *, int);
int check_size(char *);
void create_child(char *, char *);
int max(int,int);
int main(int argc, char **argv)
{
  int listenfd, connfd, udpfd, select_no, maxfd,p;
  fd_set readfs;
  socklen_t clilen;
  struct sockaddr_in cliAddr, servAddr;
  struct timeval to;
  struct message txMsg;
  char *ip,port[10],rxMsg[MAX_SIZE],cli_ip[10][50],cli_fname[10][70];
  int cnt=-1, cli_port[10],i;

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

  maxfd= max(listenfd,udpfd)+1;

  while(1)
  {
    to.tv_sec = 100;
    to.tv_usec = 0;
    FD_ZERO(&readfs);
    FD_SET(listenfd,&readfs);
    FD_SET(udpfd,&readfs);

    select_no = select(maxfd, &readfs,NULL,NULL,&to);
    printf("Select returns with %ld sec and %ld usec remaining\n", to.tv_sec, to.tv_usec);

    if(FD_ISSET(udpfd, &readfs))
    {
      printf("Socket selected\n");
      fflush(stdout);
      if(recvfrom(udpfd, &rxMsg, sizeof(rxMsg), MSG_DONTWAIT, (SA *)&cliAddr, &clilen))
      {
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
      	  cnt++;
      	  strcpy(cli_ip[cnt],ip);
	  strcpy(cli_fname[cnt],txMsg.fname);
      	  cli_port[cnt]=p;
	}
	else
	  send_file_udp(txMsg,udpfd,ip,p);
      }
    }

    if(FD_ISSET(listenfd, &readfs))
    {
      clilen = sizeof(cliAddr);
      connfd = accept(listenfd, (SA *)&cliAddr, &clilen);

      ip= inet_ntoa(cliAddr.sin_addr);
      //printf("%s:", ip);
      p= ntohs(cliAddr.sin_port);
      //printf("%d\n", p);

      printf("TCP connection\n");
      for(i=0;i<=cnt;i++)
      {
	  printf("\n%s comparing to %s:",ip,cli_ip[i]);
          printf("%d comparing to %d\n", p,cli_port[i]);
	if(strcmp(ip,cli_ip[i])==0 && p==cli_port[i])
	{
	  send_file_tcp(connfd,cli_fname[i]);
	  close(connfd);
	  cnt--;
	  break;
	}
      }
      
    }

    if(select_no == 0)
    {
      printf("Timeout occurred\n");
      break;
    }

    select_no = 0;
  }
  printf("REception complete\n"); 
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

void send_file_tcp(int connfd, char *fname)
{
  FILE *fp;
  char txMsg[MAX_SIZE];
  int n;

  fp= fopen(fname,"r");

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
int max(int x, int y)
{
  if(x>y)
    return x;
  else
    return y;
}
