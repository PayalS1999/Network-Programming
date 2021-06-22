#ifndef __UNP_H
#define __UNP_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/un.h>

#define MAXLINE 100 
#define LISTENQ 1024
#define SA struct sockaddr
#endif

int num_conn = 0;
struct message{
  char req[10];
  char type[5];
  char file[50];
};
