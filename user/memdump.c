#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void memdump(char *fmt, char *data, int len);

int
main(int argc, char *argv[])
{
  if (argc == 1) {
    printf("Example 1:\n");
    int a[2] = {61810, 2026};
    memdump("ii", (char *)a, sizeof(a));

    printf("Example 2:\n");
    memdump("S", "a string", sizeof("a string"));

    printf("Example 3:\n");
    char *s = "another";
    memdump("s", (char *)&s, sizeof(s));

    struct sss {
      char *ptr;
      int num1;
      short num2;
      char byte;
      char bytes[8];
    } example;

    example.ptr = "hello";
    example.num1 = 1819438967;
    example.num2 = 100;
    example.byte = 'z';
    strcpy(example.bytes, "xyzzy");

    printf("Example 4:\n");
    memdump("pihcS", (char *)&example, sizeof(example));

    printf("Example 5:\n");
    memdump("sccccc", (char *)&example, sizeof(example));
  } else if (argc == 2) {
    // format in argv[1], up to 512 bytes of data from standard input.
    char data[512];
    int n = 0;
    memset(data, '\0', sizeof(data));
    while (n < sizeof(data)) {
      int nn = read(0, data + n, sizeof(data) - n);
      if (nn <= 0)
        break;
      n += nn;
    }
    memdump(argv[1], data, n);
  } else {
    printf("Usage: memdump [format]\n");
    exit(1);
  }
  exit(0);
}

void
memdump(char *fmt, char *data, int len)
{
  char *p = data;
  char *end = data + len;

  for (char *f = fmt; *f; f++) {
    int need;
    switch (*f) {
    case 'i': need = 4; break;
    case 'p': need = 8; break;
    case 'h': need = 2; break;
    case 'c': need = 1; break;
    case 's': need = 8; break;
    case 'S':
      for (char *q = p; q < end && *q; q++)
        printf("%c", *q);
      printf("\n");
      return;
    default:
      printf("memdump: unknown format character '%c'\n", *f);
      return;
    }

    if (p + need > end) {
      printf("memdump: not enough data for '%c'\n", *f);
      return;
    }

    switch (*f) {
    case 'i': {
      int v;
      memmove(&v, p, 4);
      printf("%d\n", v);
      p += 4;
      break;
    }
    case 'p': {
      uint64 v;
      memmove(&v, p, 8);
      printf("%lx\n", v);
      p += 8;
      break;
    }
    case 'h': {
      short v;
      memmove(&v, p, 2);
      printf("%d\n", v);
      p += 2;
      break;
    }
    case 'c': {
      printf("%c\n", *p);
      p += 1;
      break;
    }
    case 's': {
      char *s;
      memmove(&s, p, 8);
      printf("%s\n", s);
      p += 8;
      break;
    }
    }
  }
}
