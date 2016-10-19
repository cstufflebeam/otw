#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>
#include <time.h>

int ReadData(int sock, char* buf,int len);
char GetStenographicChar(double delay);

int main(int argc, char** argv) {
  sockaddr_un addr;
  FILE* output=fopen("output.jpg","w");
  if(output==NULL) {
    printf("Can't open output file for writing.");
    exit(1);
  }
  addr.sun_family=AF_UNIX;
  strcpy(addr.sun_path,"/semtex/nature");
  int sock;
  if((sock=socket(AF_UNIX, SOCK_STREAM, 0))==-1) {
    printf("Failed to create socket (%d).\n",errno);
    exit(1);
  }
  int length=strlen(addr.sun_path) + sizeof(addr.sun_family);
  if(connect(sock,(sockaddr*)&addr, length)==-1) {
    printf("Failed to connect (%d).\n",errno);
    exit(1);
  }

  printf("Connected to local socket.\n");
  char buf[3];
  memset(buf,'\0',3);
  int recieved=0;
  while((recieved=ReadData(sock, buf,3))!=0) {
    fwrite(buf,sizeof(char),recieved,output);
    memset(buf,'\0',3);
  }
  printf("Done. Check output.jpg.\n");
  return 0;
}


char GetStenographicChar(double delay) {
  if(delay<1.0)
    return 0;
  else if(delay>=1.0 && delay<2.0)
    return 'Q';
  else if (delay>=2.0 && delay<3.0)
    return 'L';
  else if (delay>=3.0 && delay<4.0)
    return 'A';
  else if(delay>=4.0 && delay<5.0)
    return 'V';
}

int ReadData(int sock, char* buf,int len) {
  time_t theTime=time(NULL);
  int recieved=recv(sock,buf,1,0);
  if(recieved==-1) {
    printf("recv returned error (%d)\n",errno);
    return 0;
  }
  double delay=difftime(time(NULL),theTime);
  if(delay>=1.0) {
    buf[1]=buf[0];
    buf[0]=GetStenographicChar(delay);
    recieved++;
  }
  return recieved;
}
