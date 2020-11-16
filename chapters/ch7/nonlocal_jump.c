#include <limits.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

jmp_buf jmpbuffer;

void do_line(char*);
void cmd_add(void);
char* get_token(void);

int main(void) {
  char line[LINE_MAX];

  printf("usage: <command> <arg>...\ne.g. add 1 2\n");

  if (setjmp(jmpbuffer) != 0) {
    printf("please input command again: ");
  }

  while (fgets(line, LINE_MAX, stdin) != NULL) {
    do_line(line);
  }

  exit(0);
}

char* tok_ptr; /* global pointer for get_token() */
char token[LINE_MAX];

void do_line(char* ptr) /* process one line of input */
{
  char* cmd;

  tok_ptr = ptr;
  while ((cmd = get_token())) {
    if (strcmp(cmd, "add") == 0) {
      cmd_add();
    } else {
      printf("unexpected token: %s\n", cmd);
      longjmp(jmpbuffer, 1);
    }
  }
}

void cmd_add(void) {
  char* token;
  int a, b;

  token = get_token();

  if (!token) {
    printf("missing arguments\n");
    longjmp(jmpbuffer, 1);
  }

  a = atoi(token);

  if (a == 0 && token[0] != '0') {
    printf("parse error: %s is not an integer\n", token);
    longjmp(jmpbuffer, 1);
  }

  token = get_token();

  if (!token) {
    printf("missing arguments\n");
    longjmp(jmpbuffer, 1);
  }

  b = atoi(token);

  if (b == 0 && token[0] != '0') {
    printf("parse error: %s is not an integer\n", token);
    longjmp(jmpbuffer, 1);
  }

  printf("%d\n", a + b);
}

char* get_token(void) {
  /* fetch next token from line pointed to by tok_ptr */
  int i = 0;

  while (*tok_ptr) {
    if ((*tok_ptr == ' ' || *tok_ptr == '\n')) {
      tok_ptr++;
      if (i) {
        token[i] = '\0';
        break;
      }
    } else {
      token[i++] = *tok_ptr++;
    }
  }

  if (!(*tok_ptr)) {
    token[i] = '\0';
  }

  if (i) {
    return token;
  } else {
    return NULL;
  }
}