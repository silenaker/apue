#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "error.h"

void prt_usage() {
  printf("usage: <program> <path>\n");
  exit(1);
}

int main(int argc, char* argv[]) {
  DIR* curdirp;
  struct dirent* ent;
  char curdir[1024];

  if (argc < 2) {
    prt_usage();
  }

  if (chdir(argv[1]) < 0) {
    err_sys("chdir failed");
  }
  printf("chdir to %s succeeded\n", argv[1]);

  if (!(curdirp = opendir("."))) {
    if (!getcwd(curdir, 10)) {
      err_sys("getcwd failed");
    }
    err_sys("cannot open current working directory: %s", curdir);
  }

  while ((ent = readdir(curdirp))) {
    printf("%s\n", ent->d_name);
  }

  exit(0);
}