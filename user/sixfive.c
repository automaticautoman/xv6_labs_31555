#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static void
check(int n)
{
  if(n > 0 && (n % 5 == 0 || n % 6 == 0))
    printf("%d\n", n);
}

static void
process(int fd)
{
  char c;
  int cur = 0;
  int has_digits = 0;

  while(read(fd, &c, 1) == 1){
    if(c >= '0' && c <= '9'){
      cur = cur * 10 + (c - '0');
      has_digits = 1;
    } else {
      if(has_digits){
        check(cur);
        cur = 0;
        has_digits = 0;
      }
    }
  }
  if(has_digits)
    check(cur);
}

int
main(int argc, char *argv[])
{
  if(argc == 1){
    process(0);
  } else {
    for(int i = 1; i < argc; i++){
      int fd = open(argv[i], O_RDONLY);
      if(fd < 0){
        fprintf(2, "sixfive: cannot open %s\n", argv[i]);
        continue;
      }
      process(fd);
      close(fd);
    }
  }
  exit(0);
}
