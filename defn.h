#define LINELEN 1024
#define NOWAIT 0
#define WAIT 1
#define NOEXPAND 0
#define EXPAND 2

#include <stdbool.h>
int expand(char *orig, char *new, int newsize);
bool isBuiltIn(char** argv, int argc);
int processline(char* line, int infd, int outfd, int flags);
