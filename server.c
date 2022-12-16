#include "lib/inet_wrap.h"
#include "clients.c"
#include "lib/util.h"
#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>

#define forever for (;;)
#define NAME_SIZE 32
#define BUFF_SIZE 1024
#define MAX_CLIENTS 100

int check_args(int argc, char *argv[]);
int accept_clients(int socketfd);


int main (int argc, char *argv[]) {
  sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);

  if (check_args(argc, argv) == -1) return EXIT_FAILURE;

  short port = atoi(argv[1]);

  int socketfd;

  //for setsockopt
  int option = 1;

  SAI serv_addr;
  // SAI must be filled with zeros
  bzero(&serv_addr, sizeof(serv_addr));

  //serv_addr.sin_addr.s_addr = inet_addr(localhost);
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // host to network short cuz of network endian
  serv_addr.sin_port = htons(port);

  // address family ipv4
  serv_addr.sin_family = AF_INET;

  socketfd = _socket(AF_INET, SOCK_STREAM, 0);

  // to reuse port and address of this server
  _setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option));

  _bind(socketfd, (SA *)&serv_addr, sizeof(serv_addr));
  _listen(socketfd, LISTENQ);

  printf("Server has successfully started...\n");

  pthread_t pid = 0;
  pthread_create(&pid, NULL, print_clients, NULL);

  if (accept_clients(socketfd) == -1) {
    perror("Error occured");
    return 1;
  }

  return 0;
}

int check_args(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>", argv[0]);
    return -1;
  }
  return 0;
}

int _add_client(Client *client) {
  if (add_client(client) == -1) {
    char buff[BUFF_SIZE];
    inet_ntop(client->sockfd, &client->addr.sin_addr, buff, sizeof(buff));
    printf("Maximum clients connected. Rejected connection from: %s\n", buff);
    _close(client->sockfd);
    return -1;
  }

  return 0;
}

int accept_clients(int socketfd) {
  // load_from_database();
  SAI client_addr;
  bzero(&client_addr, sizeof(client_addr));
  socklen_t client_len = sizeof(client_addr);

  pthread_t pid;
  forever { 
    int connfd = _accept(socketfd, (SA *)&client_addr, &client_len);
    printf("Accepted\n");

    Client *client = create_client(&client_addr, connfd);
    if (_add_client(client) == -1) continue;

    pthread_create(&pid, NULL, &handle_client, (void *)client);
  }

  return EXIT_SUCCESS;
}
