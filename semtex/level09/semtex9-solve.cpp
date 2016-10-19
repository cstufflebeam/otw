#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>


#define IP_ICMP           1
#define ICMP_SYN        121
#define ICMP_ACK        122
#define ICMP_DATA       123
#define ICMP_CON        124
#define ICMP_RESET      125


class CICMPPacket {
public:
  CICMPPacket(int sock);
  ~CICMPPacket();
  void SendPacket();
  void SetPacketType(int);
  void SetPacketPayload(char);//ICMP_DATA only
  char GetPacketPayload();
  void SetPacketDestination(char*);
  void SetPacketRaw(char*);

  icmphdr* icmp;
  ip* iph;
  void PrintPacket();
private:
  char* payload;
  unsigned short CheckSum(unsigned short* buf, int nwords);
  int m_iSocket;
  void BuildPacket();
  char m_sPacket[500];
  unsigned int m_iPacketLength;
  sockaddr_in m_hAddr;

};

class CICMPClient {
public:
  CICMPClient(int sock);
  ~CICMPClient();
  void ConnectTo(char*);
  int SendData(char*,int);
  int RecvData(char*,int);
  pid_t MainListenLoop();
  void Broadcast(int);
private:
  pid_t m_iPid;
  int m_iTimeout;
  CICMPPacket ReadPacket();
  int m_iSocket;
  CICMPPacket syn;
  CICMPPacket data;
};




/************************************************
 *ICMP Packet Definitions
 ************************************************/

CICMPPacket::CICMPPacket(int sock) : m_iSocket(sock) {
  memset(m_sPacket,'\0',500);
  iph=(ip*)m_sPacket;
  icmp=(icmphdr*)(m_sPacket+sizeof(ip));
  payload=(char*)(m_sPacket+sizeof(ip)+sizeof(icmphdr));
}

CICMPPacket::~CICMPPacket() {

}

void CICMPPacket::SendPacket() {
  BuildPacket();
  int one=1;
  const int* val=&one;
  if(setsockopt(m_iSocket,IPPROTO_IP,IP_HDRINCL,val,sizeof(one))<0) {
    printf("[-] Call to hdrincl failed - kernel will insert it's own headers!\n");
    exit(1);
  }
  if(sendto(m_iSocket,m_sPacket,m_iPacketLength,0,(sockaddr*)&m_hAddr,sizeof(m_hAddr))<0) {
    printf("[-] Failed to send data.\n");
    exit(1);
  }
  //  printf("[+] Packet Sent.\n");
}

void CICMPPacket::SetPacketRaw(char* packet) {
  memcpy(m_sPacket,packet,500);
}

void CICMPPacket::SetPacketType(int type) {
  icmp->type=type;
}

void CICMPPacket::SetPacketPayload(char pl) {
  if(icmp->type!=ICMP_DATA)
    printf("[i] Warning: icmp->type!=ICMP_DATA. Payload will be ignored.\n");
  //  char* ptr=m_sPacket+sizeof(ip)+sizeof(icmphdr)+1;//+1 ?
  //  *ptr=payload;
  *payload=pl;
}

char CICMPPacket::GetPacketPayload() {
  return *payload;
}

void CICMPPacket::SetPacketDestination(char* dst) {
  m_hAddr.sin_addr.s_addr=inet_addr(dst);
  //iph->ip_dst.s_addr is set in BuildPacket

}


void CICMPPacket::BuildPacket() {
  m_hAddr.sin_family=AF_INET;
  
  iph->ip_hl=5;
  iph->ip_v=4;
  iph->ip_tos=0;
  m_iPacketLength=sizeof(ip)+sizeof(icmphdr)+1;
  iph->ip_len=htons(m_iPacketLength);
  iph->ip_id=htonl(12345);//shouldn't matter
  iph->ip_off=0;
  iph->ip_ttl=255;
  iph->ip_p=IP_ICMP;
  iph->ip_src.s_addr=inet_addr("69.55.233.89");//semtex.labs.pulltheplug.org
  iph->ip_dst.s_addr=m_hAddr.sin_addr.s_addr;
  iph->ip_sum=CheckSum((unsigned short*)m_sPacket,m_iPacketLength);
  
  if(!icmp->type) 
    printf("[i] Warning: icmp->type==NULL. Sending anyways.\n");
  icmp->code=0;
  icmp->un.echo.sequence=123;
  icmp->un.echo.id=123;
  icmp->checksum=CheckSum((unsigned short*)icmp,sizeof(icmphdr)+1);

  //done.
}


unsigned short CICMPPacket::CheckSum (unsigned short *buf, int nwords)
{
  unsigned long sum;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}

void CICMPPacket::PrintPacket() {
  printf("[");
  for(unsigned int i=0;i<500;i++) 
    printf("%x",m_sPacket[i]);
  printf("]\n");
}





/************************************************
 *ICMP Client Definitions
 ************************************************/

CICMPClient::CICMPClient(int sock) : m_iSocket(sock),syn(sock),data(sock) {

}

CICMPClient::~CICMPClient() {

}

void CICMPClient::Broadcast(int type) {
  int one=1;
  if(setsockopt(m_iSocket,SOL_SOCKET,SO_BROADCAST,&one,sizeof(one))<0) {
    printf("[-] Failed to set SO_BROADCAST on socket.");
  }
  CICMPPacket broadcast(m_iSocket);
  broadcast.SetPacketType(type);
  broadcast.SetPacketDestination("69.55.233.0");
  broadcast.SendPacket();
  

}
void CICMPClient::ConnectTo(char* dst) {
  syn.SetPacketType(ICMP_SYN);
  syn.SetPacketDestination(dst);
  syn.SendPacket();
  //now we need to wait for ICMP_ACK
  
  CICMPPacket response=ReadPacket();
  if(response.icmp->type!=ICMP_ACK && response.icmp->type!=ICMP_CON) {//if we're already connected, just reuse the connection.
    printf("[-] Server responded in an invalid manner (%d).\n",response.icmp->type);
    exit(1);
  }
  data.SetPacketType(ICMP_DATA);
  data.SetPacketDestination(dst);
  printf("[+] ICMP Connection to server complete.\n");
}

CICMPPacket CICMPClient::ReadPacket() {
  char buffer[500];
  memset(buffer,'\0',500);

  int bytesRead=recv(m_iSocket,buffer,500,0);
  if(bytesRead==-1) {
    printf("Call to read failed (%d)\n",errno);
    exit(1);
  }
  CICMPPacket response(m_iSocket);
  response.SetPacketRaw(buffer);
  if(strcmp(inet_ntoa(response.iph->ip_src),"69.55.233.89")==0) {//recieved our own packet over lo
    printf("Ignoring packet from 127.0.0.1\n");
    return ReadPacket();
  }
  //  printf("[i] Recieved %d bytes from %s\n",bytesRead,inet_ntoa(response.iph->ip_src));
  //  response.PrintPacket();
  switch(response.icmp->type) {
  case ICMP_ACK:
    printf("[+] Server responded with ACK.\n");
    break;
  case ICMP_SYN:
    printf("[-] Server sent us ICMP_SYN for some reason.\n");
    exit(1);
    break;
  case ICMP_DATA:
    //    printf("[+] Recieved data (%x [%c]) from %s.\n",response.GetPacketPayload(),response.GetPacketPayload(),inet_ntoa(response.iph->ip_src));
    break;
  case ICMP_CON:
    printf("[i] Warning: Server responded with ICMP_CON (Already connected).\n");
    break;
  case ICMP_RESET:
    printf("[-] Server responded with ICMP_RESET (not connected/connection reset)\n");
    exit(1);
    break;
  default:
    printf("[i] Recieved an unknown type of packet (%d) from %s\n",response.icmp->type,inet_ntoa(response.iph->ip_src));
    break;
  }
  return response;
}



int CICMPClient::RecvData(char* buffer, int len) {
  m_iTimeout=false;
  unsigned int count=0;
  //  while(!m_iTimeout && count<len) {
    CICMPPacket response=ReadPacket();
    if(response.icmp->type==ICMP_DATA) {
      buffer[count++]=response.GetPacketPayload();
    }
    else {
      //not for us
    }
    //  }
  return 0;
}

int CICMPClient::SendData(char* buffer,int len) {
  for(int i=0;i<len;i++) {
    data.SetPacketPayload(buffer[i]);
    data.SendPacket();
  }
  return 0;
}


pid_t CICMPClient::MainListenLoop() {
  if((m_iPid=fork())==0) {
    while(1) {
      char buffer[1024];
      memset(buffer,'\0',1024);
      RecvData(buffer,1024);
      printf("%s",buffer);
      fflush(NULL);
    }
  }
  return m_iPid;
}

int main(int argc, char** argv) {
  if(argc!=2) {
    printf("Syntax: /semtex/rawrapper %s\n",argv[0]);
    exit(1);
  }
  CICMPClient client(atoi(argv[1]));
  //  client.Broadcast(ICMP_SYN);
  client.ConnectTo("69.55.233.87");
  client.MainListenLoop();//listen for responses from the server
  printf("Client initialization complete. Listening for user input:\n");
  while(1) {
    char buffer[1024];
    memset(buffer,'\0',1024);
    std::cin.getline(buffer,1024);
    client.SendData(buffer,strlen(buffer));
    client.SendData("\n",1);
    fflush(NULL);
  }
  return 0;
}
