#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/fs.h"

// Regex helpers (copied from user/grep.c).
int match(char *, char *);
int matchhere(char *, char *);
int matchstar(int, char *, char *);

static char*
basename(char *path)
{
  char *p;
  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  return p + 1;
}

// NEW: -exec state
static int do_exec = 0;
static char *exec_cmd = 0;
static char **exec_args = 0;
static int exec_argc = 0;

// NEW: handle a match (print, or fork+exec)
static void
handle_match(char *path)
{
  if(!do_exec){
    printf("%s\n", path);
    return;
  }

  char *argv[MAXARG];
  int n = 0;
  argv[n++] = exec_cmd;
  for(int i = 0; i < exec_argc; i++)
    argv[n++] = exec_args[i];
  argv[n++] = path;
  argv[n] = 0;

  int pid = fork();
  if(pid < 0){
    fprintf(2, "find: fork failed\n");
    return;
  }
  if(pid == 0){
    exec(exec_cmd, argv);
    fprintf(2, "find: exec %s failed\n", exec_cmd);
    exit(1);
  }
  wait(0);
}

static void
find(char *path, char *target)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    // CHANGED: regex match instead of strcmp
    if(match(target, basename(path)))
      handle_match(path);
    break;

  case T_DIR:
    // CHANGED: regex match instead of strcmp
    if(match(target, basename(path)))
      handle_match(path);

    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
      fprintf(2, "find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      find(buf, target);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "usage: find path name [-exec cmd args...]\n");
    exit(1);
  }

  if(argc > 3){
    if(strcmp(argv[3], "-exec") != 0 || argc < 5){
      fprintf(2, "usage: find path name [-exec cmd args...]\n");
      exit(1);
    }
    do_exec   = 1;
    exec_cmd  = argv[4];
    exec_args = &argv[5];
    exec_argc = argc - 5;
  }

  find(argv[1], argv[2]);
  exit(0);
}

// ===== Regex matcher (from grep.c, K&P "The Practice of Programming") =====

int
match(char *re, char *text)
{
  if (re[0] == '^')
    return matchhere(re + 1, text);
  do {
    if (matchhere(re, text))
      return 1;
  } while (*text++ != '\0');
  return 0;
}

int
matchhere(char *re, char *text)
{
  if (re[0] == '\0')
    return 1;
  if (re[1] == '*')
    return matchstar(re[0], re + 2, text);
  if (re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if (*text != '\0' && (re[0] == '.' || re[0] == *text))
    return matchhere(re + 1, text + 1);
  return 0;
}

int
matchstar(int c, char *re, char *text)
{
  do {
    if (matchhere(re, text))
      return 1;
  } while (*text != '\0' && (*text++ == c || c == '.'));
  return 0;
}
