#include "defn.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

extern char** argv2;
extern int argc2;
extern int num;
extern int prevcmd;

int expand(char *orig, char *new, int newsize) {
  char* j = new;
  char* i = orig;
  char* newstring;
  char* k;
  char* p;
  int inc = 0;
  int argnum = 0;
  int counter = 0;
  int num1 = 0;
  int size = 0;
  char* var;
  while (*i !='\0' && size <= newsize) {
    if (*i == '$') {

      i++;
      if (*i == '{') {
        i++;
        var = i;
        while (*i != '}') {
          if (*i == '\0') {
            fprintf(stderr, "Error: No closing bracket found. \n");
            //return 1 if unsuccessful
            return 1;
          }
          i++;
        }
        *i = '\0';
        newstring = getenv(var);
        *i = '}';
        i++;
        if (newstring!=NULL) {
          k = newstring;
          while (*k != '\0' && size != newsize) {
            *j = *k;
            j++;
            size++;
            k++;
          }
        }
      }
      //else if there's a '$$', replace it with pid
      else if (*i == '$') {
        num1 = sprintf(j, "%d", getpid());
        j+= num1;
        size+= num1;
        i++;
      }
      else if (*i == '#') {
        if (argc2 > 1) {
          inc = sprintf(j, "%d", (argc2-1)-num);
          j += inc;
          size += inc;

        }
        else {
          inc = sprintf(j, "%d", argc2);
          j += inc;
          size += inc;
        }
        i++;
      }
      else if (*i >= '0' && *i <= '9') {
        char arr[LINELEN];
        while (*i >= '0' && *i <= '9') {
          arr[counter] = *i;
          i++;
          counter++;
        }
        arr[counter] = '\0';
        counter = 0;
        argnum = atoi(arr);
        if (argnum+num >= argc2-1) {
        }

        else {
          if (argc2 < 2) {
            p = argv2[0];
          }
          else {
            if (argnum == 0) {
              p = argv2[1];
            }
            else {
              p = argv2[num+argnum+1];
            }
          }

          while (*p != '\0' && size <= newsize) {
            *j = *p;
            p++;
            j++;
            size++;
          }
        }
      }
      else if (*i == '?') {
        int inc = sprintf(j, "%d", prevcmd);
        j+= inc;
        size += inc;
        i++;
      }
      else if (*i == '(') {
        int retval = 0;
        int status = 0;
        int pipe1[2];
        char* pip = i+1;
        char* ptr = i;
        char* j2 = j;
        int inc = 0;
        int opening = 1;
        int closing = 0;
        ptr++;
        while ((opening != closing) && (*ptr != '\0')) {
          if (*ptr == '(') {
            opening++;
          }
          if (*ptr == ')') {
            closing++;
          }
          ptr++;
        }
        i = ptr;
        *(ptr-1) = '\0';
        if (opening != closing) {
          perror("Number of opening parenthises does not match number of closing parenthises ");
          return 1;
        }
        if (pipe(pipe1)==-1) {
          perror("Error creating pipe. ");
          return 1;
        }
        retval = processline(pip, 0, pipe1[1], NOWAIT | EXPAND);
        close(pipe1[1]);
        while ((inc = read(pipe1[0], j, newsize-size)) && (size < newsize)) {
          j+= inc;
          size += inc;
        }
        if (*(j-1) == '\n') {
          j = j - 1;
          size = size - 1;
        }
        while (j2 != j) {
          if (*j2 == '\n') {
            *j2 = ' ';
          }
          j2++;
        }
        close(pipe1[0]);
        if (retval > 1) {
          waitpid(retval, &status, 0);
          if (WIFEXITED(status)) {
            prevcmd = WEXITSTATUS(status);
          }
          if (WIFSIGNALED(status)) {
            prevcmd = 128 + WTERMSIG(status);
            if (SIGINT!=WTERMSIG(status)) {
              printf("Process was killed by a %s", strsignal(WTERMSIG(status)));
            }
            if (WCOREDUMP(status)) {
              printf("%s", " (Core dumped) \n");
            }
            else {
              printf("\n");
            }
          }
        }

        }
        else {
          *j = '$';
          j++;
          size++;
          *j = *i;
          i++;
          j++;
          size++;
        }

      }
      else if (*i == '*' && *(i-1) == ' ') {
        if (*(i+1)==' ' || *(i+1)=='\0') {
          DIR* dir;
          struct dirent* direntry;
          char* counter2;
          dir = opendir(".");
          direntry = readdir(dir);
          //get the file
          while (direntry != NULL) {
            counter2 = direntry->d_name;
            if (*counter2 != '.') {
              //copy to new line
              while (*counter2 != '\0' && size <= newsize) {
                *j = *counter2;
                j++;
                counter2++;
                size++;
              }
              *j = ' ';
              j++;
            }
            direntry = readdir(dir);
          }
          closedir(dir);
          i++;
        }
        else {
          DIR* dir;
          struct dirent* direntry;
          int c3 = 0;
          int c4 = 0;
          char* jpos = j;
          char* c6;
          char* c5;
          int flength = 0;
          int newlen = 0;
          char temp[1024];
          char* counter2;
          dir = opendir(".");
          direntry = readdir(dir);
          //get the file
          i++;
          while (*i != ' ' && *i != '\0') {
            temp[c3] = *i;
            i++;
            c3++;
          }
          temp[c3] = '\0';
          while (direntry != NULL) {
            counter2 = direntry->d_name;
            if (*counter2 != '.') {
              //copy to new line
              flength = strlen(counter2);
              newlen = flength - c3;
              if (newlen >= 0) {
                while (counter2[newlen] != '\0') {
                  if (counter2[newlen]==temp[c4]) {
                    newlen++;
                    c4++;
                  }
                  else {
                    break;
                  }
                }
                //copying
                c5 = counter2;
                while ((*c5 != '\0' && size <= newsize) && (counter2[newlen]=='\0')) {
                  *j = *c5;
                  j++;
                  c5++;
                  size++;
                }
                if (counter2[newlen]=='\0') {
                  *j = ' ';
                  j++;
                }
              }
            }
            direntry = readdir(dir);
            c4 = 0;
          }
          c6 = temp;
          while (*c6 != '\0') {
            if (*c6 == '/') {
              fprintf(stderr, "Error: invalid input. \n");
              return 1;
            }
            c6++;
          }

          if (j == jpos) {
            c5 = temp;
            while (*c5 != '\0') {
              *j = *c5;
              c5++;
              j++;
              size++;
            }
          }
          closedir(dir);
        }
      }

      else {
        *j = *i;
        j++;
        size++;
        i++;
      }
    }
    *j = '\0';
    if (newsize < size) {
      fprintf(stderr, "Error: overflow. \n");
      return 1;
    }
    return 0;
  }
