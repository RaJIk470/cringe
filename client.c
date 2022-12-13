#include "lib/inet_wrap.h"
#include "lib/util.h"
#include <pthread.h>
#include <stdio.h>

#define BUFF_SIZE 1024

typedef struct {
  SAI serv_addr;
  int socketfd;
} Data;

void read_from_server(void *arg);
void read_user_input(void *arg);
int check_args(int argc, char *argv[]);

int main (int argc, char *argv[]) {
  if (check_args(argc, argv) == -1) {
    printf("Usage: %s <ip> <port>\n", argv[0]);
    return 1;
  }
  
  int socketfd;
  socketfd = _socket(AF_INET, SOCK_STREAM, 0);
 
  SAI serv_addr;
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(argv[2]));
  char *ipv4_addr = argv[1];

  _inet_pton(AF_INET, ipv4_addr, &serv_addr.sin_addr);

  _connect(socketfd, (SA *)&serv_addr, sizeof(serv_addr));
  
  Data data;
  data.socketfd = socketfd;
  data.serv_addr = serv_addr;
  pthread_t pid;

  pthread_create(&pid, NULL, (void *)read_from_server, (void *)&data);
  pthread_create(&pid, NULL, (void *)read_user_input, (void *)&socketfd);

  pthread_join(pid, NULL);

  _close(socketfd);

  return 0;
}

int check_args(int argc, char *argv[]) {
  if (argc != 3)
    return -1;

  return 0;
}

void read_from_server(void *arg) {
  Data *data = (Data *)arg;
  int socketfd = data->socketfd;
  SAI serv_addr = data->serv_addr;

  int n;
  char buff[BUFF_SIZE];

  while ((n = read(socketfd, buff, BUFF_SIZE)) > 0) {
    buff[n] = '\0';
    printf("%s", buff);
  }
}

void read_user_input(void *arg) {
  int *socketfd = (int *)arg;
  char message[BUFF_SIZE];

  
  while (fgets(message, BUFF_SIZE, stdin) > 0) {
    _write(*socketfd, message, BUFF_SIZE);
    if (startsWith("/exit", message))
      break;
  }
}
