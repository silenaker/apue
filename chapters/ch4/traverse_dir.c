#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "error.h"

/* function type that is called for each filename */
typedef void Myfunc(const char*);

static void myfunc(const char* filename);
static void myftw(const char*, Myfunc);
static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    err_quit("usage: ftw <starting-pathname>");
  }

  myfunc(argv[1]);
  ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;

  if (ntot == 0)
    ntot = 1; /* avoid divide by 0; print 0 for all counts */

  printf("regular files = %7ld, %5.2f %%\n", nreg, nreg * 100.0 / ntot);
  printf("directories = %7ld, %5.2f %%\n", ndir, ndir * 100.0 / ntot);
  printf("block special = %7ld, %5.2f %%\n", nblk, nblk * 100.0 / ntot);
  printf("char special = %7ld, %5.2f %%\n", nchr, nchr * 100.0 / ntot);
  printf("FIFOs = %7ld, %5.2f %%\n", nfifo, nfifo * 100.0 / ntot);
  printf("symbolic links = %7ld, %5.2f %%\n", nslink, nslink * 100.0 / ntot);
  printf("sockets = %7ld, %5.2f %%\n", nsock, nsock * 100.0 / ntot);
}

static void myfunc(const char* filename) {
  struct stat statbuf;

  puts(filename);

  if (lstat(filename, &statbuf) < 0) {
    err_sys("stat error for %s", filename);
  }

  switch (statbuf.st_mode & S_IFMT) {
    case S_IFREG:
      nreg++;
      break;
    case S_IFBLK:
      nblk++;
      break;
    case S_IFCHR:
      nchr++;
      break;
    case S_IFIFO:
      nfifo++;
      break;
    case S_IFLNK:
      nslink++;
      break;
    case S_IFSOCK:
      nsock++;
      break;
    case S_IFDIR:
      ndir++;
      return myftw(filename, myfunc);
    default:
      err_dump("unknown type for filename %s", filename);
  }
}

static void myftw(const char* pathname, Myfunc func) {
  DIR* dp;
  struct dirent* dirp;
  char filepath[PATH_MAX];

  if ((dp = opendir(pathname)) == NULL) {
    err_sys("can't read directory %s", pathname);
  }

  while ((dirp = readdir(dp)) != NULL) {
    if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0) {
      continue; /* ignore dot and dot-dot */
    }
    snprintf(filepath, PATH_MAX, "%s/%s", pathname, dirp->d_name);
    func(filepath);
  }

  if (closedir(dp) < 0) {
    err_sys("can't close directory %s", pathname);
  }
}