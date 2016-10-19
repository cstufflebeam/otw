#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>


unsigned short checksum (unsigned short *buf, int nwords)
{
  unsigned long sum;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}


int main(int argc, char** argv) {
  if(argc!=2) {
    printf("Usage: /semtex/rawrapper %s",argv[0]);
    exit(1);
  }
  int socket=atoi(argv[1]);

  char packet[500];
  memset(packet,'\0',500);
  ip* iph=(ip*)packet;
  icmphdr* icmp=(icmphdr*)(packet+sizeof(ip));

  char payload[]="defamai";
  strcpy(packet+sizeof(ip)+sizeof(icmphdr),payload);

  sockaddr_in addr;
  addr.sin_family=AF_INET;
  addr.sin_addr.s_addr=inet_addr("69.55.233.87");//brebera.labs.pulltheplug.org

  printf("Setting up ip header (%d).\n",sizeof(ip));
  iph->ip_hl=5;
  iph->ip_v=4;//ipv4
  iph->ip_tos=0;
  iph->ip_len=htons(sizeof(ip)+sizeof(icmphdr)+strlen(payload));
  iph->ip_id=htonl(12345);
  iph->ip_off=0;
  iph->ip_ttl=255;
  iph->ip_p=1;
  iph->ip_src.s_addr=inet_addr("198.81.129.100"); //cia.gov
  iph->ip_dst.s_addr=addr.sin_addr.s_addr;
  iph->ip_sum=checksum((unsigned short*)packet,35);

  printf("IPH Checksum: %x\nSetting up icmp header (%d)\n",iph->ip_sum,sizeof(icmphdr));
  icmp->type=130;
  icmp->code=0;
  icmp->un.echo.sequence=123;
  icmp->un.echo.id=123;
  icmp->checksum=checksum((unsigned short*)icmp,sizeof(icmphdr)+strlen(payload));

  printf("ICMP Checksum: %x\nSending packet:",icmp->checksum);
  for(unsigned int i=0;i<35;i++)
    printf("%x",packet[i]);


  int one=1;//ugly hack to enable hdrincl
  const int* val=&one;
  if(setsockopt(socket,IPPROTO_IP,IP_HDRINCL,val,sizeof(one))<0) {
    printf("[-] Call to hdrincl failed - kernel will insert it's own headers!\n");
    exit(1);
  }

  if(sendto(socket,packet,35,0,(sockaddr*)&addr,sizeof(addr))<0) {
    printf("[-] Failed to send data!\n");
    exit(1);
  }
  printf("\nAll done.\n");
}
