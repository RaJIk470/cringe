#ifndef STRUCTURES_GUARD
#define STRUCTURES_GUARD

#include "lib/inet_wrap.h"
#include "lib/util.h"

#define NAME_SIZE 32
#define MAX_CHATS 1000
#define MAX_CLIENTS 100

typedef struct {
  struct sockaddr_in addr;
  int sockfd;
  int uid;
  int chat_id;
  char name[NAME_SIZE];
} Client;


typedef struct {
  int id;
  char name[NAME_SIZE];
} Chat;
#endif 
