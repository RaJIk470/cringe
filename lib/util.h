#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

char *trim_whitespaces(char *str);
int starts_with(const char *prefix, const char *str);
int readln(int fd, char *buffer, size_t n);
void remove_new_lines(char *str, int len);
