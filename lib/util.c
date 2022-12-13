#include "util.h"

char *trim_whitespaces(char *str) {
  char *end;
  while ((*str) == ' ')
    str++;
  if (*str == 0) return str;
  end = str + strnlen(str, 128) - 1;
  while (end > str && (*end) == ' ') end--;
  *(end + 1) = '\0';
  return str;
}

int startsWith(const char *restrict prefix, const char *restrict str) {
  while(*prefix) {
    if(*prefix++ != *str++)
      return 0;
  }

  return 1;
}

void remove_new_lines(char *str, int len) {
  for (int i = 0; i < len; i++) {
    if (str[i] == 10) {
      str[i] = '\0';
    }
  }
}

int readln(int fd, char *buffer, size_t n) {
  while (read(fd, buffer, n) > 0) {

  }
  return 0;
}
