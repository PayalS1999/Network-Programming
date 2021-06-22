#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include "unp.h"
#include "server_chat_grp.h"

#define size 4
int main(int argc, char *argv[])
{
  int sockfd;
  struct sockaddr_in servAddr, cliAddr; 
  socklen_t cliAddrLen;

  struct message m1;
  struct group g[size];
  //FILE *fp;

  int n,cnt[size],i,port,j;

  char str[MAX_SIZE],p[10];

  for(i=0;i<size;i++)
    cnt[i]=-1;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  bzero(&servAddr, sizeof(servAddr));
  bzero(&m1,sizeof(m1));
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

  printf("%d: ",8877);
  inet_ntop(AF_INET, &servAddr.sin_addr, str, 40);
  printf("%s",str);
  fflush(stdout);
  while(1)
  {
    n= recvfrom(sockfd, &m1, sizeof(m1), 0, (SA *)&cliAddr, &cliAddrLen);
    printf("\n Bit recieved: %d\n",n);
    //fwrite(m1.msg,n,1,stdout);
    fflush(stdout);

    bzero(&str, sizeof(str));
    if (strcmp(m1.msg,"join\n")==0)
    {
    	cnt[m1.grp]++;

    	inet_ntop(AF_INET, &cliAddr.sin_addr, g[m1.grp].ip[cnt[m1.grp]], 40);

    	port= ntohs(cliAddr.sin_port);
    	g[m1.grp].port[cnt[m1.grp]]= port;
	strcpy(g[m1.grp].user[m1.grp][cnt[m1.grp]], m1.user);

	if(m1.grp!=0)
	{
		cnt[0]++;
		inet_ntop(AF_INET, &cliAddr.sin_addr, g[0].ip[cnt[0]], 40);
		g[0].port[cnt[0]]= port;
		strcpy(g[0].user[0][cnt[0]], m1.user);

	}
    	snprintf(p,sizeof(p),"%d",g[m1.grp].port[cnt[m1.grp]]);
	
    	printf("\n join request : %s: %s : %s",g[m1.grp].ip[cnt[m1.grp]],p,g[m1.grp].user[m1.grp][cnt[m1.grp]]);
    	fflush(stdout);

    	strcpy(str,"SUCCESS: Active users: \n");
	for(i=0;i<=cnt[m1.grp];i++)
	{
	strcat(str,g[m1.grp].user[m1.grp][i]);
	strcat(str,"\n");
	}

    	sendto(sockfd, &str, sizeof(str),0, (SA *)&cliAddr, cliAddrLen);

   }

   else if (strcmp(m1.msg,"leave\n")==0)
   {

	if(m1.grp==0)
	{

	   for(j=0;j<10;j++)
	   {	
	   	for(i=0;i<=cnt[j];i++)
	   	{
	   	    if(strcmp(g[j].user[j][i],m1.user)==0)
	   	    {
		  	if(i==cnt[j])
		    	cnt[j]--;
		  	else
		  	{
		    	  strcpy(g[j].ip[i],g[j].ip[cnt[j]]);
		    	  g[m1.grp].port[i]= g[j].port[cnt[j]];
		    	  strcpy(g[j].user[j][i],g[j].user[j][cnt[j]]);
		    	  cnt[j]--;
		  	}
	   	    }
	        }
	    }
	}

	else
	{

	   bzero(&cliAddr, sizeof(cliAddr));
  	   cliAddr.sin_family = AF_INET;
	   cliAddrLen = sizeof(cliAddr);


	   for(i=0;i<=cnt[m1.grp];i++)
	   {
	     if(strcmp(g[m1.grp].user[m1.grp][i],m1.user)==0)
	     {

		inet_pton(AF_INET, g[m1.grp].ip[i], &cliAddr.sin_addr);
		cliAddr.sin_port = htons(g[m1.grp].port[i]);
		strcpy(str,"You left the group no.:");
		snprintf(p,sizeof(p),"%d",m1.grp);
		strcat(str,p);
		
		sendto(sockfd, &str, sizeof(str),0, (SA *)&cliAddr, cliAddrLen);

		if(i==cnt[m1.grp])
		   cnt[m1.grp]--;
		else
		{
		   strcpy(g[m1.grp].ip[i],g[m1.grp].ip[cnt[m1.grp]]);
		   g[m1.grp].port[i]= g[m1.grp].port[cnt[m1.grp]];
		   strcpy(g[m1.grp].user[m1.grp][i],g[m1.grp].user[m1.grp][cnt[m1.grp]]);
		   cnt[m1.grp]--;
		}
	      }
	   }
	}
   }

    else
    {

	inet_ntop(AF_INET, &cliAddr.sin_addr, str, 40);
	strcat(str,": ");
	snprintf(p,sizeof(p),"%d",cliAddr.sin_port);
	strcat(str,p);

	if(m1.grp==10)
   	{
	    bzero(&cliAddr, sizeof(cliAddr));
  	    cliAddr.sin_family = AF_INET;
	    cliAddrLen = sizeof(cliAddr);

	    strcat(str,"\npersonnel message : \n");
	    strcat(str,"\nFrom user:  ");
	    strcat(str,m1.user);
	    strcat(str,"    \n");    
	    strcat(str,m1.msg);
	    strcpy(m1.msg,str);
	    
	    for(i=0;i<=cnt[0];i++)
	    {
		if(strcmp(g[0].user[0][i], m1.user)==0)
		{
		   inet_pton(AF_INET, g[0].ip[i], &cliAddr.sin_addr);
		   cliAddr.sin_port = htons(g[0].port[i]);
		   break;
		}
	    }

	    sendto(sockfd, &m1.msg, n,0, (SA *)&cliAddr, cliAddrLen);
   	}

    	else
        {

	    if(m1.grp!=0)
	    {
		for(i=0;i<=cnt[m1.grp];i++)
		{
		    if(strcmp(g[m1.grp].user[m1.grp][i], m1.user)==0)
			break;
		}
		if(i==cnt[m1.grp]+1)
		{
		  strcpy(str,"SORRY YOU CAN'T SEND MESSAGE TO THE GROUP. ");
		  sendto(sockfd, &str, n,0, (SA *)&cliAddr, cliAddrLen);
		  continue;
		}
	    }

	    if(m1.grp==0)
	    {
	        strcat(str,"\nBROADCAST message: \n");
		strcat(str,"\nFrom user:  ");
		strcat(str,m1.user);
		strcat(str,"    \n");
	    }

	    else
		{
		   strcat(str,"\nGROUP ");
		   snprintf(p,sizeof(p),"%d",m1.grp);
		   strcat(str,p);
		   strcat(str,"\nFrom user:  ");
		   strcat(str,m1.user);
		   strcat(str,"\n message: \n");
		}
	    strcat(str,m1.msg);
	    strcpy(m1.msg,str);

	    for(i=0;i<=cnt[m1.grp];i++)
	    {

		if(strcmp(g[m1.grp].user[m1.grp][i],m1.user)==0)
		  continue;

		bzero(&cliAddr, sizeof(cliAddr));
  	    	cliAddr.sin_family = AF_INET;
	    	cliAddrLen = sizeof(cliAddr);

	    	inet_pton(AF_INET, g[m1.grp].ip[i], &cliAddr.sin_addr);
  	    	cliAddr.sin_port = htons(g[m1.grp].port[i]);
	    
      	    	sendto(sockfd, &m1.msg, n,0, (SA *)&cliAddr, cliAddrLen);
	    }
	}
    }
  }

}
