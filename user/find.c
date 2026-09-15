#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/fs.h"

static char*
basename(char *path)
{
  char *p;
  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  return p + 1;
}

// -exec state (populated by main).
static int do_exec = 0;
static char *exec_cmd = 0;
static char **exec_args = 0;
static int exec_argc = 0;

// Called for every match: either print, or fork+exec the command with the path appended.
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
    if(strcmp(basename(path), target) == 0)
      handle_match(path);
    break;

  case T_DIR:
    if(strcmp(basename(path), target) == 0)
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

  // Optional -exec cmd args...
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
