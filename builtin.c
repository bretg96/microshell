#include "defn.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

extern int num;
extern int argc2;
extern int prevcmd;

int exitShell(char** argv, int argc) {
  if (argc < 2) {
    exit(0);
  }
  else {
  exit(atoi(argv[1]));
  return 0;
  }
}

int envset(char** argv, int argc) {
  int val = 0;
  if (argc < 3) {
    perror("Error: Not enough arguments. ");
    return 1;
  }
  val = setenv(argv[1], argv[2], 1);
  if (val == -1) {
    perror("Error: Environment set failed. ");
    return 1;
  }
  else {
    return 0;
  }
}

int envunset(char** argv, int argc) {
  int val = 0;
  if (argc < 2) {
    perror("Error: Not enough arguments. ");
    return 1;
  }
  val = unsetenv(argv[1]);
  if (val == -1) {
    perror("Error: Environment unset failed. ");
    return 1;
  }
  else {
    return 0;
  }
}

int cd(char** argv, int argc) {
  int val = 0;
  if (argc > 2) {
    perror("Error: Too many arguments. ");
    return 1;
  }
  if (argc == 1) {
    val = chdir(getenv("HOME"));
  }
  else {
    val = chdir(argv[1]);
  }
  if (val == -1) {
    perror("Error: Change directory failed. ");
    return 1;
  }
  else {
    return 0;
  }
}

int shift(char** argv, int argc) {
  char* i = argv[1];
  int j = 0;
  if ((num + atoi(argv[1])) >= (argc2-1)) {
  perror("Error: Cannot shift more than # of arguments. ");
  return 1;
}
  if (argc < 2) {
    num+=1;
  }
  else {
  while (*i != '\0') {
    if (!(*i >= '0' && *i <= '9')) {
      perror("Error: Not a valid entry. ");
      return 1;
    }
    i++;
  }
  num = num + atoi(argv[1]);
  }
  return 0;
  //add error checking for shift overflow (more args than we have)
  //subtract num from acgc2
  //for unshift, just reset num back to 0
}

int unshift (char** argv, int argc) {
  int val = 0;
  if (argc > 2) {
    perror("Error: Invalid syntax. ");
    return 1;
  }
  if (argc < 2) {
    num = 0;
  }
  else {
    val = atoi(argv[1]);
    if (val > num) {
      perror("Error: Cannot unshift more than what has been shifted. ");
      return 1;
    }
    num-=val;
  }
  return 0;
}

int sstat (char** argv, int argc) {
  int i = 1;
  struct stat buf;
  if (stat(argv[1], &buf) < 0) {
    return 1;
  }
  while (i < argc) {
    printf("File Size: \t\t%d bytes\n",buf.st_size);
    printf("Number of Links: \t%d\n",buf.st_nlink);
    printf("File inode: \t\t%d\n",buf.st_ino);
    i++;
}
  return 0;
}

bool isBuiltIn(char** argv, int argc) {
  if (*argv == NULL) {
    return false;
  }
  char* commands[] = {"exit", "envset", "envunset", "cd", "shift", "unshift", "sstat"};
  int (*ptrs[])(char** argv, int argc) = {exitShell, envset, envunset, cd, shift, unshift, sstat};
  int cmdNum = 7;
  for (int i = 0; i < cmdNum; i++) {
    if (strcmp(argv[0], commands[i])==0) {
      prevcmd = ptrs[i](argv, argc);

      return true;
    }
  }
  return false;
}
