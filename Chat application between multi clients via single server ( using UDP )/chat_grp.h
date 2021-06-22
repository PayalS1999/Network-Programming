#define MAX_SIZE 1000

struct group{
  int port[10];
  char user[30];
  char ip[10][40];
};

struct message{
  int grp;
  char user[30];
  char msg[MAX_SIZE];
};

