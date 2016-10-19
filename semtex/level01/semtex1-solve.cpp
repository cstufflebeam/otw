#include <stdlib.h>
#include <iostream>
#include <stdio.h>

char* encrypt13(char*);
char* reverse13(char*);
char* reverse(char*);
char transition(char);

int main(int argc, char** argv) {
  if(argc>1)
    printf("Encrypted String: %s. Unencrypted String: %s.\n",argv[1],reverse13(argv[1]));
  else
    printf("Usage: %s (encrypted string).\n",argv[0]);
  return 0;
}

char* reverse13(char* encrypted) {
  char* buf=new char[strlen(encrypted)];
  char* prevbuf=new char[strlen(encrypted)];
  memset(buf,'\0',strlen(encrypted));
  memset(prevbuf,'\0',strlen(encrypted));
  strcpy(buf,encrypted);
  strcpy(prevbuf,encrypted);

  
  for(unsigned int i=0;i<99;i++) {
    buf[0]=transition(prevbuf[10]);
    buf[1]=transition(prevbuf[11]-1);
    buf[2]=transition(prevbuf[12]-1);
    buf[3]=transition(prevbuf[1]-1);
    buf[4]=transition(prevbuf[2]-2);
    buf[5]=transition(prevbuf[3]-2);
    buf[6]=transition(prevbuf[4]-3);
    buf[7]=transition(prevbuf[5]-4);
    buf[8]=transition(prevbuf[6]-5);
    buf[9]=transition(prevbuf[7]-7);
    buf[10]=transition(prevbuf[8]-9);
    buf[11]=transition(prevbuf[9]-12);
    buf[12]=transition(prevbuf[0]-16);
    strcpy(prevbuf,buf);
    printf("%s\n",buf);
  }
  return buf;
}


char* encrypt13(char* encrypted) {
  char* buf=new char[strlen(encrypted)];
  char* prevbuf=new char[strlen(encrypted)];
  memset(buf,'\0',strlen(encrypted));
  memset(prevbuf,'\0',strlen(encrypted));
  strcpy(buf,encrypted);
  strcpy(prevbuf,encrypted);

  
  for(unsigned int i=0;i<99;i++) {
    buf[10]=transition(prevbuf[0]);
    buf[11]=transition(prevbuf[1]+1);
    buf[12]=transition(prevbuf[2]+1);
    buf[1]=transition(prevbuf[3]+1);
    buf[2]=transition(prevbuf[4]+2);
    buf[3]=transition(prevbuf[5]+2);
    buf[4]=transition(prevbuf[6]+3);
    buf[5]=transition(prevbuf[7]+4);
    buf[6]=transition(prevbuf[8]+5);
    buf[7]=transition(prevbuf[9]+7);
    buf[8]=transition(prevbuf[10]+9);
    buf[9]=transition(prevbuf[11]+12);
    buf[0]=transition(prevbuf[12]+16);
    strcpy(prevbuf,buf);
    printf("%s\n",buf);
  }
  return buf;
}


char transition(char x) {
  int dif=x-'Z';
  if(x>'Z')
    return 'A'+dif-1;
  if(x<'A') 
    return 'Z'+(x-'A')+1;
  return x;
}
