#include <stdio.h>
#include <unistd.h>
#include "unp.h"

void communicate(int sockfd, char*, char*);
void child_proc_term_hdlr(int);

int main(int argc, char **argv)
{
  int listenfd, connfd, to_exit = 0,p;
  pid_t childpid;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;
  char *ip,port[10];

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(argv[1]));

  if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
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

  //signal (SIGCHLD, child_proc_term_hdlr);
   for(;;) 
  { 
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
    if(connfd>0)
    {
    printf("Client IP address and port number is :\n");
    ip= inet_ntoa(cliaddr.sin_addr);
    printf("%s:", ip);
    p= ntohs(cliaddr.sin_port);
    printf("%d\n", p);
    snprintf(port,sizeof(port),"%d",p);
    
    num_conn ++;

       if((childpid = fork()) == 0)
      { 
        printf("listenfd = %d, connfd =  %d\n", listenfd, connfd);
        close(listenfd);
        communicate(connfd,ip,port);
        close(connfd);
        printf("Connection closed with %s : %s",ip,port);
        fflush(stdout);
	exit(0);
      }
      else
      {
	close(connfd);
      }
    }
  }
}

void communicate(int sockfd, char *ip, char *port)
{
  int i,j,cnt;
  FILE *fp;
  ssize_t n, m, buf_sz;
  char send_buff[MAXLINE],fname[25],str[25],recv_line[MAXLINE],*line=NULL;
  char *args[] = {"./prog.sh", NULL, NULL,NULL};
  char files[10][20],*token,*p;
  struct message msg;

    fflush(stdout);
    n = recv(sockfd, &msg, sizeof(msg),0);
    if(n > 0)
    {
      cnt=0;
      char *newline=strchr(msg.file,'\n');
      if(newline)
	*newline=0;

      token=strtok(msg.file," ");
      strcpy(files[cnt],token);
      cnt++;

      while(token!=NULL)
      {
	token= strtok(NULL," ");
	if(token==NULL)
		break;
	strcpy(files[cnt],token);
	cnt++;
      }
      for(j=0;j<cnt;j++)
      {
    	if(strcmp(msg.req,"GET")==0) 
    	{
      	  strcpy(fname,files[j]);
      	  if(strcmp(msg.type,"-b")==0)
      		strcat(fname,".bin");
      	  else if(strcmp(msg.type,"-a")==0)
		strcat(fname,".txt");

      	  fp = fopen(fname, "r");

      	  if(fp==NULL)
      	  {
		strcpy(send_buff,"SORRY NO SUCH FILE EXISTS");
		send(sockfd, send_buff, sizeof(send_buff), 0);
      	  }
      	  else
      	  {
		printf("\nSending file to the client: %s:%s \n",ip,port);
       		i = 0;
      		while(1)
      		{
            	  memset(send_buff,0x0, MAXLINE+1);
            	  m = getline(&line, &buf_sz, fp);
            	  if (m < 1)
	    	  {
	        	send(sockfd, send_buff, m, 0);
	  		printf("EOF seen\n");
	  		fclose(fp);
	  		break;
	    	  }
	    	  strcpy(send_buff,line);
            	  send(sockfd, send_buff, m, 0);
            	  i++;
         	}
       		printf("%d buffers sent over socket\n", i);
		fflush(stdout);
      	  }
       }

       else if(strcmp(msg.req,"PUT")==0)
       {
    	strcpy(fname,ip);
    	strcat(fname,":");
    	strcat(fname,port);

	strcpy(str,files[j]);
      	if(strcmp(msg.type,"-b")==0)
      	  strcat(str,".bin");
      	else if(strcmp(msg.type,"-a")==0)
	  strcat(str,".txt");

	fp = fopen(str, "w");
	printf("\nRecieving file from the client: %s:%s \n",ip,port);
	while(1)
    	{
     	   if(n= recv(sockfd, recv_line, MAXLINE,0) < MAXLINE)
      	   {
		fputs(recv_line, fp);
        	printf("completed transfer file: %s\n",str);
		fclose(fp);
		fflush(stdout);
        	break; 
      	   }

      	   fputs(recv_line, fp);
      	   bzero(recv_line, MAXLINE);
    	}

	args[1] = (char *)malloc(30*sizeof(char));
        memset(args[1], 0x0, 30);

        args[2] = (char *)malloc(20*sizeof(char));
        memset(args[1], 0x0, 20);

	strcpy(args[1],fname);
	strcpy(args[2],str);

	if(fork()==0)
	{
	  if(execvp("./prog.sh", args) == -1)
            fprintf(stderr, "Failed to execute bash program\n");
	}
	else
	{
	  wait(NULL);
	}

     }
    }
   }
}

void child_proc_term_hdlr(int signo)
{
  pid_t pid;
  int stat;

  pid = waitpid(pid, &stat, WNOHANG);
  printf("Child %d terminated\n", pid);
  num_conn --;
  if(num_conn == 0)
  {
    printf("All sockets disconnected..\n");
    exit(0); 
  }
}
