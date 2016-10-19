//Compile this with gcc -fPIC -rdynamic -g -c -Wall semtex2-solve.c -fno-pie -fno-stack-protector;gcc -shared -WI,-soname,intercept.so.1 -o intercept.so.1.0.1 interc.o -lc -ldl
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

uid_t geteuid(void) {
  return 666;
}

