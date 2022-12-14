#include "lib/util.h"
#include "lib/inet_wrap.h"
#include "structures.c"


Chat *chats[MAX_CHATS];
int chat_count = 0;

Chat *add_chat(char *name) {
  Chat *chat = (Chat *)malloc(sizeof(Chat));
  chat->id = chat_count;
  strcpy(chat->name, name);

  chats[chat_count++] = chat;
  return chat;
}

Chat *find_chat_by_name(char *name) {
  for (int i = 0; i < MAX_CHATS; i++) {
    if (chats[i] != NULL && strcpy(chats[i]->name, name) == 0) {
      return chats[i];
    }
  }  

  return NULL;
}

Chat *find_chat_by_id(int id) {
  for (int i = 0; i < MAX_CHATS; i++) {
    if (chats[i] != NULL && chats[i]->id == id) {
      return chats[i];
    }
  }  

  return NULL;
}

int chat_exists_by_id(int id) {
  if (id == -1) return 0;
  for (int i = 0; i < MAX_CHATS; i++) {
    if (chats[i] != NULL && chats[i]->id == id) {
      return 1; 
    }
  }  

  return 0;
}

int chat_exists_by_name(char *name) {
  for (int i = 0; i < MAX_CHATS; i++) {
    if (chats[i] != NULL && strcpy(chats[i]->name, name) == 0) {
      return 1; 
    }
  }  

  return 0;
}

