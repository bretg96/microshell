/* CS 352 -- Miro Shell!
*
*   Sept 21, 2000,  Phil Nelson
*   Modified April 8, 2001
*   Modified January 6, 2003
*   Modified January 8, 2017
*
*/
#include "defn.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdbool.h>


/* Constants */

#define LINELEN 1024

/* Prototypes */

int processline(char* line, int infd, int outfd, int flags);

char** arg_parse (char *line, int *argcptr);

bool comment(char* buf);

/* Shell main */
pid_t sig;
int prevcmd = 0;
char** argv2;
int argc2;
int num;

struct sigact {
        sigset_t   sa_mask;
        int        sa_flags;
        void     (*sa_restorer)(void);
    };

main (int argc, char** argv)
{
  num = 0;
  argc2 = argc;
  argv2 = argv;
  char   buffer [LINELEN];
  int    len;
  FILE* readtype;
  struct sigact s;
  if (argv[1] != NULL) {
    FILE *fd = fopen(argv[1], "r");
    if (fd == NULL) {
      perror("Error: Could not open file.");
      exit(127);
    }
    readtype = fd;
  }
  else {
    readtype = stdin;
  }

  while (1) {

    /* prompt and get line */
    if (readtype == stdin) {
      fprintf (stderr, "%% ");
    }
    if (fgets (buffer, LINELEN, readtype) != buffer)
    break;


    /* Get rid of \n at end of buffer. */
    len = strlen(buffer);
    if (buffer[len-1] == '\n')
    buffer[len-1] = 0;

    comment(buffer);

    /* Run it ... */
    processline (buffer, 0, 1, WAIT | EXPAND);

  }

  if (!feof(readtype))
  perror ("read");

  return 0;		/* Also known as exit (0); */
}


int processline (char *line, int infd, int outfd, int flags)
{
  int mp[2];
  pipe(mp);
  printf ("fd = %d\n", mp[0]);
  close(mp[0]);
  close(mp[1]);
  char input[LINELEN];
  int err = 0;
  int pipecount = 0;
  char** argv;
  pid_t  cpid;
  int    status;
  int argc = 0;
  if (EXPAND & flags) {
    err = expand(line, input, LINELEN);
  }
  else {
    strcpy(input, line);
  }
  char* i = input;
  if (err == 1) {
    return 1;
  }
  while (*i != '\0') {
    if (*i == '|') {
      pipecount++;
      *i = '\0';
    }
    i++;
  }
  //check if we need to pipe
  if (pipecount > 0) {
    int temp[2];
    char* pipereader = input;
    //create the pipes
    int pipe1[2];
    int pipe2[2];
    if (pipe(pipe1) != 0) {
      fprintf(stderr, "Error piping\n");
      return 1;
    }
    if (pipe(pipe2) != 0) {
      fprintf(stderr, "Error piping\n");
      return 1;
    }
    //for the first time we see a |
    processline(input, infd, pipe1[1], NOEXPAND | NOWAIT);
    close(pipe1[1]);
    while (pipecount-1 > 0) {
      while (*pipereader != '\0') {
        pipereader++;
      }
      pipereader++;
      //call processline for each command before the last one
      processline(pipereader, pipe1[0], pipe2[1], NOEXPAND | NOWAIT);
      close(pipe1[0]);
      pipe(pipe1);
      close(pipe2[1]);
      //swap the pipes each iteration
      temp[0] = pipe1[0];
      temp[1] = pipe1[1];
      pipe1[0] = pipe2[0];
      pipe1[1] = pipe2[1];
      pipe2[0] = temp[0];
      pipe2[1] = temp[1];
      pipecount--;
    }
    while (*pipereader != '\0') {
      pipereader++;
    }
    pipereader++;
    //the last time we call processline, we use outfd
    pid_t cpid = processline(pipereader, pipe1[0], outfd, NOEXPAND | NOWAIT);
    close(pipe1[0]);
    if (flags & WAIT) {
      sig = cpid;
      //wait for the child to finish
      if (waitpid (cpid, &status, 0) < 0) {
        /* Wait wasn't successful */
        perror ("wait");
      }
      sig = 0;
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
    return cpid;
  }

  argv = arg_parse(input, &argc);
  if (isBuiltIn(argv, argc) == true) {
    free(argv);
    return 0;
  }
  /* Start a new process to do the job. */
  if (argc == 0) {
    free(argv);
    return 1;
  }
  if (argc != 0) {
    cpid = fork();
    if (cpid < 0) {
      /* Fork wasn't successful */
      //      if (argv[0])
      perror ("fork");
      return 1;
    }

    /* Check for who we are! */
    if (cpid == 0) {
      /* We are the child! */
      if (outfd != 1) {
        dup2(outfd,1);
      }
      if (infd != 0) {
        dup2(infd,0);
      }
      execvp (argv[0], argv);
      /* execlp reurned, wasn't successful */
      perror ("exec");
      fclose(stdin);  // avoid a linux stdio bug
      exit (127);
    }
  }
  free(argv);
  /* Have the parent wait for child to complete */
  if (flags & WAIT) {
    sig = cpid;
    if (waitpid (cpid, &status, 0) < 0) {
      /* Wait wasn't successful */
      perror ("wait");
    }
    sig = 0;
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
  return cpid;

}

char** arg_parse (char *line, int *argcptr) {
  //set up the pointer
  bool open = false;
  int count = 0;
  bool space = true;
  int len = strlen(line);
  for (int i = 0; i < len; i++ ) {
    if (line[i] == ' ') {
      space = true;
    }
    else {
      if (space == true && open == false) {
        count++;
      }
      if (line[i] == '"') {
        open = !open;
      }
      space = false;

    }
  }

  //populate the pointer
  char** arg = malloc((count+1)*sizeof(char*));
  int j = 0;
  char* write = line;
  char* read = line;
  space = true;
  open = false;
  while (*read != '\0') {

    if (space == true && open == false && *read != ' ') {
      arg[j] = write;
      j++;
    }
    if (*read == '"') {
      read++;
      open = !open;
      space = false;
    }
    else {
      if (*read == ' ') {
        space = true;
        if (open == false) {
          *write = '\0';
          *read = '\0';
        }
      }
      else {
        space = false;
      }
      *write = *read;
      write++;
      read++;
    }
  }
  *write = '\0';
  arg[count] = NULL;
  *argcptr = count;
  return arg;

}

bool comment(char* buf) {
  char* i = buf;
  if (*i == '#') {
    *i = '\0';
    return true;
  }
  while (*i != '\0') {
    if (*i == '#') {
      if (*(i-1) != '$') {
        *i = '\0';
        break;
      }
    }
    i++;
  }
  return false;
}
