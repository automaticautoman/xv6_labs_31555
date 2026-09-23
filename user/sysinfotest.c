#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  struct sysinfo info;

  if(sysinfo(&info) < 0){
    printf("sysinfo failed\n");
    exit(1);
  }

  printf("free memory: %ld bytes\n", info.freemem);
  exit(0);
}
