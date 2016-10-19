#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string>
#include <fstream>
#include "proxy_lib.h"

struct proxyInfo {
  std::string ip;
  unsigned int port;
};


int main(int argc, char** argv) {
  std::ifstream file;
  file.open("list.txt");
  char semtex5pass[]={"HELICOTRMA"};
  char        myID[]={"BURGERFLIP"};
  proxyInfo proxyList[50];

  for(unsigned int i=0;!file.eof();i++) {
    proxyInfo p;
    file >> p.ip >> p.port;
    proxyList[i] = p;
  }

  CProxyOut* proxy[15];
  errno=0;
  for(unsigned int i=0;i<12;i++) {
    try {
      char buf[1024];
      memset(buf,'\0',1024);
      proxy[i]=new CProxyOut();
      proxy[i]->SetProxy(proxyList[i].ip.c_str(),proxyList[i].port);
      proxy[i]->ConnectToHost("semtex.labs.pulltheplug.org",24027);
      proxy[i]->Read(buf,1024);
      printf("[i] recieved buf: %s.\n",buf);
      for(unsigned int j=0;j<12;j++) {
	buf[j]=semtex5pass[j]^buf[j];
	printf("%x,",buf[j]);
      }
      printf("\n[i] Sending back %s - %s.\n",buf,myID);
      proxy[i]->Send(buf,20);
      proxy[i]->Send(myID,strlen(myID));
      memset(buf,'\0',1024);
    }
    catch (CProxyException* e) {
      printf("[-] Exception Caught: %s (Errno: %d)\n",e->what(),errno);
    }
  }
  for(unsigned int i=0;i<12;i++) {
    if(fork()==0) {
      try {
	char buf[1024];
	memset(buf,'\0',1024);
	proxy[i]->Read(buf,1024);
	printf("[i] Recieved back %s.\n",buf);
      }
      catch (CProxyException* e) {
	printf("[-] Recieved exception: %s.\n",e->what());
      }
      break;
    }
  }
  return 0;
}

