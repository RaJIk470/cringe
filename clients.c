#include "lib/inet_wrap.h"
#include "lib/util.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#define NAME_SIZE 32
#define MAX_CLIENTS 100
#define BUFF_SIZE 1024

typedef struct {
  SAI addr;
  int sockfd;
  int uid;
  char name[NAME_SIZE];
} Client;

Client *clients[MAX_CLIENTS];
void *print_clients() {
  sleep(1);
  for (int i = 0; i < MAX_CLIENTS; i++)
    if (clients[i] != NULL)
      printf("%d.%d ", i, clients[i]->uid);
  
  printf("\n");
  return print_clients();
}
int client_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

long long uid = 1;

Client *create_client(SAI *client_addr, int sockfd) {
  Client *client = (Client *)malloc(sizeof(Client));
  client->addr = *client_addr;
  client->sockfd = sockfd;
  client->uid = uid++;
  return client;
}

void free_client(Client **client) {
  free(*client);
  *client = NULL;
}

int add_client(Client *client) {
  pthread_mutex_lock(&mutex);

  if (client_count >= MAX_CLIENTS)  {
    return -1;
  }

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] == NULL) {
      clients[i] = client;
      client_count++;
      break;
    }
  }

  pthread_mutex_unlock(&mutex);

  return 0;
}

int remove_client(int uid) {
  pthread_mutex_lock(&mutex);

  if (client_count == 0) {
    return -1;
  }

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] != NULL && clients[i]->uid == uid) {
      free_client(&clients[i]);
      client_count--;
      break;
    }
  }

  pthread_mutex_unlock(&mutex);

  return 0;
}


int send_to_all(char *message, int sender_uid) {
  pthread_mutex_lock(&mutex);

  if (client_count == 0) {
    return -1;
  }

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] != NULL && clients[i]->uid != sender_uid) {
      _write(clients[i]->sockfd, message, strnlen(message, BUFF_SIZE));
    }
  }

  pthread_mutex_unlock(&mutex);

  return 0;
}

void cleanup(void *data) {
  Client *client = (Client *)data;
  char message[BUFF_SIZE];
  snprintf(message, BUFF_SIZE, "%s has leaved\n", client->name); 
  send_to_all(message, client->uid);
  remove_client(client->uid);
}

int check_if_exit(char *buff, Client *client) {
  if (startsWith("/exit", buff)) {
    _write(client->sockfd, "I'm Rajik!\n", BUFF_SIZE);
    _close(client->sockfd);
    remove_client(client->uid);
    return -1;
  }

  return 0;
}

int read_with_exit_check(Client *client, char *buff) {
  int res = read(client->sockfd, buff, BUFF_SIZE);
  if (check_if_exit(buff, client) == -1)
    pthread_cancel(pthread_self());
  return res;
}

int fd_is_valid(int fd) {
  return fcntl(fd, F_GETFD) != 1 || errno != EBADF;
}

void handle_logining(Client *client) {
  char buff[BUFF_SIZE];
  char name[NAME_SIZE];

  int res;
  _write(client->sockfd, "/login <username> to login\n/reg <username> <password> to register\n", BUFF_SIZE);
  if (errno == EPIPE) return;

  read_with_exit_check(client, buff);
  char *token = strtok(buff, " ");  
  char *username = strtok(NULL, " "); 
  if (username == NULL) {
    _write(client->sockfd, "Empty username. Try again\n", BUFF_SIZE);
    if (errno == EPIPE) return;
    handle_logining(client);
    return;
  }

  if (startsWith("/login", buff)) {
    remove_new_lines(username, NAME_SIZE);
    strncpy(client->name, username, NAME_SIZE);
    snprintf(buff, BUFF_SIZE, "Hello... ughm... not a bad name honestly... I'm just...Hello, %s\n", client->name); 
    printf("buff: %s\n", buff);
    _write(client->sockfd, buff, BUFF_SIZE);
    if (errno == EPIPE) return;
    // try to find in database and so on...
    return;
  }

  char *password = strtok(NULL, " "); 
  if (password == NULL) {
    res = _write(client->sockfd, "Empty password. Try again\n", BUFF_SIZE);
    if (res == -1) return;
    handle_logining(client);
    return;
  }

  if (startsWith("/reg", buff)) {
    // try to save in database and so on...
    return;
  }

  handle_logining(client);
}

void *handle_client(void *arg) {
  Client *client = (Client *)arg;
  handle_logining(client);

  char buff[BUFF_SIZE];
  char message_to_send[BUFF_SIZE];
  while (read_with_exit_check(client, buff) > 0) {
    printf("message from %s: %s\n", client->name, buff);
    snprintf(message_to_send, BUFF_SIZE, "%s: %s", client->name, buff);
    send_to_all(message_to_send, client->uid);
  }
  
  cleanup(client);
  return NULL;
}
