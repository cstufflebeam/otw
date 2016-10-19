#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/user.h>
#include <sys/syscall.h>
#include <errno.h>

void printLocation(pid_t);

char* lookupPtraceErrno() {
  switch(errno) {
  case EFAULT:
    return "efault";
    break;
  case EBUSY:
    return "ebusy";
  case EIO:
    return "eio";
  case EPERM:
    return "eperm";
  }
  return NULL;
}
int main(int argc, char** argv) {
  long originalEax,eax;
  char* run[2];
  int status;
  int insyscall=0;
  long params[3];
  pid_t pid=fork();
  long desired=2005;
  struct user_regs_struct regs;

  if(!pid) {
    if(ptrace(PTRACE_TRACEME,0,NULL,NULL)==-1) {
      printf("PTRACE_TRACEME\n");
      exit(1);
    }
    run[0]="/semtex/pass";
    run[1]=0x00;
    execve(run[0],run,NULL);
  }
  else {
    while(1) {
      wait(&status);
      if(WIFEXITED(status))
	break;
      originalEax=ptrace(PTRACE_PEEKUSER,pid,4*ORIG_EAX,NULL);
      if(originalEax==SYS_geteuid32) {
	if(insyscall==0) {
	  insyscall=1;
	}
	else {
	  ptrace(PTRACE_GETREGS,pid,0,&regs);
	  regs.eax=2005;
	  ptrace(PTRACE_SETREGS,pid,0,&regs);
	  eax=ptrace(PTRACE_PEEKUSER,pid,4*EAX,NULL);
	  printf("geteuid32 returned with %ld\n",eax);
	  insyscall=0;

	}
      }
      ptrace(PTRACE_SYSCALL,pid,NULL,NULL);
    }
  }
  return 0;
}

