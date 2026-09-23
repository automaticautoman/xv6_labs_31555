#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int total = 4096 * 128;
  char *mem = sbrk(total);
  if(mem == (char *)0xffffffffffffffff){
    printf("sbrk failed\n");
    exit(1);
  }

  for(int i = 0; i < total; i++){
    if((mem[i] >= 'a' && mem[i] <= 'z') ||
       (mem[i] >= 'A' && mem[i] <= 'Z') ||
       (mem[i] >= '0' && mem[i] <= '9')){
      int j = i;
      int has_digit = 0, has_upper = 0;
      while(j < total &&
            ((mem[j] >= 'a' && mem[j] <= 'z') ||
             (mem[j] >= 'A' && mem[j] <= 'Z') ||
             (mem[j] >= '0' && mem[j] <= '9'))){
        if(mem[j] >= '0' && mem[j] <= '9') has_digit = 1;
        if(mem[j] >= 'A' && mem[j] <= 'Z') has_upper = 1;
        j++;
      }
      int len = j - i;
      if(len >= 6 && len <= 12 && has_digit && has_upper){
        write(1, &mem[i], len);
        write(1, "\n", 1);
      }
      i = j - 1;
    }
  }

  exit(0);
}
