#include <iostream>
#include <fstream>


int main(int argc, char** argv) {
  char* rawbuf;
  FILE* fp=fopen("/tmp/semtex0","w");
  FILE* rawdata=fopen("rawdata","r");
  if(!fp or !rawdata) {
    printf("Failed to open files.\n");
    exit(1);
  }
  fseek (rawdata , 0 , SEEK_END);
  long rawsize = ftell (rawdata);
  rewind (rawdata);
  rawbuf = new char[rawsize];
  fread(rawbuf,1,rawsize,rawdata);
  char* ptr=rawbuf;
  for(unsigned int i=0;i<rawsize;i++,ptr++) {
    if(i%2==0)
      fwrite(ptr,1,1,fp);
  }
}
