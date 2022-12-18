#include "lib/inet_wrap.h"
#include "lib/util.h"
#include "structures.c"
#include "chats.c"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFF_SIZE 1024 

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
  client->chat_id = -1;
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

Client *find_client_by_name(char *name) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0)
      return clients[i];
  }

  return NULL; 
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

int send_to_all_in_chat(char *message, Client *sender) {
  pthread_mutex_lock(&mutex);

  if (sender->chat_id == -1) {
    _write(sender->sockfd, "You're not in the chat\n", BUFF_SIZE);
    pthread_mutex_unlock(&mutex);
    return -1;
  }

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] != NULL && clients[i]->uid != sender->uid && clients[i]->chat_id == sender->chat_id) {
      _write(clients[i]->sockfd, message, strnlen(message, BUFF_SIZE));
    }
  }

  pthread_mutex_unlock(&mutex);

  return 0;
}

void cleanup(void *data) {
  Client *client = (Client *)data;
  char message[BUFF_SIZE];
  if (client->chat_id != -1) {
    snprintf(message, BUFF_SIZE, "%s has leaved\n", client->name); 
    printf("%s", message);
    send_to_all_in_chat(message, client);
  }
  remove_client(client->uid);
}

int check_if_exit(char *buff, Client *client) {
  if (starts_with("/exit", buff)) {
    _close(client->sockfd);
    remove_client(client->uid);
    return -1;
  }

  return 0;
}

int read_with_exit_check(Client *client, char *buff) {
  int res = read(client->sockfd, buff, BUFF_SIZE);
  char *b = buff;
  while (*b != 0) {
    printf("%d, %c;    ", *b, *b);
    b++;
  }
  printf("\n");
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
  _write(client->sockfd, "/login <username> to login\n", BUFF_SIZE);
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

  if (starts_with("/login", buff)) {
    remove_new_lines(username, NAME_SIZE);
    strncpy(client->name, username, NAME_SIZE);
    snprintf(buff, BUFF_SIZE, "Hello, %s\n", client->name); 
    _write(client->sockfd, buff, BUFF_SIZE);
    if (errno == EPIPE) return;
    return;
  }

  handle_logining(client);
}

int writestr(int sockfd, const char *str)
{
  int n, len = strlen(str);
  while (len > 0)
  {
    n = _write(sockfd, str, len);
    if (n < 0) return n;
    str += n;
    len -= n;
  }
  return 0;
}

char command_list[] = 
  "/create_chat <chat_name> - create a chat\n"
  "/join_chat <chat_id> - join to some chat\n"
  "/leave_chat - leave current chat\n"
  "/msg <username> <message> - send message to some user\n"
  "/list_chats - show all available chats\n"
  "/list_users - show all users\n";

void handle_help_command(Client *client) {
  writestr(client->sockfd, command_list);
}

void handle_create_chat_command(Client *client, char *command) {
  char *token = strtok(command, " ");
  char *name = strtok(NULL, " ");
  char message[BUFF_SIZE];

  if (name != NULL) {
    remove_new_lines(name, strlen(name));
    Chat *chat = add_chat(name);
    snprintf(message, BUFF_SIZE, "Created chat with name %s and id %d\n", chat->name, chat->id);
    writestr(client->sockfd, message);
  } else {
    snprintf(message, BUFF_SIZE, "Empty chat name\n");
    writestr(client->sockfd, message);
  }
}

void handle_join_chat_command(Client *client, char *command) {
  char *token = strtok(command, " ");
  int id = atoi(strtok(NULL, " "));

  if (!chat_exists_by_id(id)) {
    _write(client->sockfd, "Chat with such an in does not exist\n", BUFF_SIZE);
    return;
  }
  client->chat_id = id;
}

void handle_leave_chat_command(Client *client, char *command) {
  if (client->chat_id == -1) {
    _write(client->sockfd, "You're not in the chat\n", BUFF_SIZE);
    return;
  }

  char message[BUFF_SIZE];
  snprintf(message, BUFF_SIZE, "User %s just has leaved conversation =(\n", client->name);
  send_to_all_in_chat(message, client);
  client->chat_id = -1;
}

void handle_list_chats_command(Client *client, char *command) {
  for (int i = 0; i < MAX_CHATS; i++) {
    if (chats[i] != NULL) {
      char message[BUFF_SIZE];
      snprintf(message, BUFF_SIZE, "%s (%d)\n", chats[i]->name, chats[i]->id);
      _write(client->sockfd, message, BUFF_SIZE);
    }
  }
}

void handle_msg_command(Client *client, char *command) {
  char *token = strtok(command, " ");
  char *name = strtok(NULL, " ");
  char *msg = &command[strlen(token) + strlen(name) + 2];

  Client *receiver = find_client_by_name(name); 
  if (receiver == NULL) {
    _write(client->sockfd, "User with such an id does not exist\n", BUFF_SIZE);
    return;
  }

  char message_to_send[BUFF_SIZE*2];
  snprintf(message_to_send, BUFF_SIZE, "private message from %s: %s", client->name, msg);

  _write(receiver->sockfd, message_to_send, BUFF_SIZE);
}

void handle_list_users_command(Client *client, char *command) {

}

void handle_is_user_active_command(Client *client, char *command) {

}

void handle_command(Client *client, char *command) {
  if (starts_with("/help", command)) {
    handle_help_command(client);
    return;
  }

  if (starts_with("/create_chat", command)) {
    handle_create_chat_command(client, command);
    return;
  }

  if (starts_with("/join_chat", command)) {
    handle_join_chat_command(client, command);
    return;
  }

  if (starts_with("/leave_chat", command)) {
    handle_leave_chat_command(client, command);
    return;
  }

  if (starts_with("/list_chats", command)) {
    handle_list_chats_command(client, command);
    return;
  }
  
  if (starts_with("/msg", command)) {
    handle_msg_command(client, command);
    return;
  }

  if (starts_with("/list_users", command)) {
    handle_list_users_command(client, command);
    return;
  }

  if (starts_with("/is_user_active", command)) {
    handle_is_user_active_command(client, command);
    return;
  }

  _write(client->sockfd, "There is no such a command. /help to get commands\n", BUFF_SIZE);
}

void handle_message(Client *client, char *message) {
  if (message[0] == '/') {
    handle_command(client, message);  
    return;
  }

  char message_to_send[BUFF_SIZE];
  snprintf(message_to_send, BUFF_SIZE, "%s: %s", client->name, message);
  send_to_all_in_chat(message_to_send, client);
}

void *handle_client(void *arg) {
  Client *client = (Client *)arg;
  handle_logining(client);

  char buff[BUFF_SIZE];
  while (read_with_exit_check(client, buff) > 0) {
    printf("message from %s: %s\n", client->name, buff);
    handle_message(client, buff);
  }
  
  cleanup(client);
  return NULL;
}
