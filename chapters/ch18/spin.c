#include <string.h>
#include <time.h>
#include <unistd.h>

int main() {
  char frames[] = "-\\|/";
  char msg[] = "loading";
  char current_line[sizeof(msg) + 1];
  char erase_c[] = "\010 \010";  // backspace space backspace
  char erase_l[] =
      "\010 \010\010 \010\010 \010\010 \010\010 \010\010 \010\010 \010\010 "
      "\010";
  struct timespec frame_interval;

  frame_interval.tv_sec = 0;
  frame_interval.tv_nsec = 1000000000 / 24;

  for (int i = 0;; i = i % 4) {
    write(STDOUT_FILENO, erase_l, strlen(erase_l));
    current_line[0] = frames[i];
    strcpy(current_line + 1, msg);
    write(STDOUT_FILENO, current_line, strlen(current_line));
    nanosleep(&frame_interval, NULL);
    i++;
  }
}