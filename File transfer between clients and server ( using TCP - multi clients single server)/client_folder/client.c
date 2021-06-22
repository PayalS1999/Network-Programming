#include "unp.h"

struct sockaddr_in servaddr;
socklen_t clilen, servlen;

int main(int argc, char *argv[])
{
  int sockfd;
  void str_cli (int);

  if(argc < 2)
    perror("Usage : myclient <IPADDR> <PORT>");

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  if(argc == 3)
    servaddr.sin_port = htons(atoi(argv[2]));
  else
    servaddr.sin_port = htons(9110);
  inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

  if(connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0)
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
    exit(0);
  } 
  printf("To get or to put files (binary or text file) from or to the server respectively follow the syntax:\n");
  printf("<GET/PUT> <-b / -a>" "file_name\nNOTE: Make sure the file is in the folder of server to get the file or in the folder of client to put the file\n");
  str_cli(sockfd);

  exit(0);

}


void str_cli (int sockfd)
{
    char sendline[MAXLINE], recvline[MAXLINE],fname[20],*send_buff;
    struct message msg;
    FILE *fp;
    ssize_t buf_sz;
    int i,n,m,cnt;
    char files[10][20],*token;

    bzero(sendline, MAXLINE);
    bzero(recvline, MAXLINE);

    scanf("%s %s",msg.req,msg.type);
    fgets(msg.file,50,stdin);

    send(sockfd,&msg,sizeof(msg),0);

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

    for(i=0;i<cnt;i++)
    {
  	if(strcmp(msg.req,"GET")==0)
  	{
   	  strcpy(fname,files[i]);

	  if(strcmp(msg.type,"-b")==0)
      		strcat(fname,".bin");
   	  else if(strcmp(msg.type,"-a")==0)
      		strcat(fname,".txt");

   	  fp = fopen(fname, "w");
   	  printf("\nRecieving %s file to the server: \n",fname);
     	  while(1)
	  {
     	    if(n= recv(sockfd, recvline, MAXLINE,0) < MAXLINE)
      	    {
		fputs(recvline, fp);
		fclose(fp);
        	printf("completed transfer file:\n");
	    	fflush(stdout);
        	break; 
      	    }

      	    fputs(recvline, fp);
      	    bzero(recvline, MAXLINE);
          }
        }

  	else if(strcmp(msg.req,"PUT")==0)
  	{

     	  strcpy(fname,files[i]);

     	  if(strcmp(msg.type,"-b")==0)
        	strcat(fname,".bin");
     	  else if(strcmp(msg.type,"-a")==0)
        	strcat(fname,".txt");

     	  fp = fopen(fname, "r");

      	  printf("\nSending file to the server %s: \n",fname);
      	  while(1)
      	  {
        	m = getline(&send_buff, &buf_sz, fp);
        	if (m < 1)
		{
	  	   printf("EOF seen\n");
	  	   fclose(fp);
	  	   break;
		}
		strcpy(sendline,send_buff);
        	send(sockfd, sendline, m, 0);
     	  }
  	}
    }
    close(sockfd);
}



