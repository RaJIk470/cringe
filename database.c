#include "structures.c"
#include "lib/inet_wrap.h"
#include "lib/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

PGconn *conn = NULL;

void disconnect_from_db() {
  PQfinish(conn);
}

void do_exit() {
  fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
  disconnect_from_db();
  exit(1);
}

void load_chats() {
  PGresult *res = PQexec(conn, "select * from chat;");

  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    PQclear(res);
    do_exit();
  }

  int rows = PQntuples(res);

  for (int i = 0; i < rows; i++) {
    int id = atoi(PQgetvalue(res, i, 0));
    char *name = PQgetvalue(res, i, 1);

    printf("id: %d; name: %s\n", id, name);
  }

  PQclear(res);
}

int add_chat_to_db(char *name) {
  char *query = "insert into chat values($1) returning id;";
  const char *param_values[1];
  param_values[0] = name;
  PGresult *res = PQexecParams(conn, query, 1, NULL, param_values, NULL, NULL, 0);    

  
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    printf("No data retrieved\n");        
    PQclear(res);
    do_exit();
  }   

  return 1;
}

int user_exists_by_username(char *username) {
  char *query = "select * from chat_user where username=$1;";
  const char *param_values[1];
  param_values[0] = username;
  PGresult *res = PQexecParams(conn, query, 1, NULL, param_values, NULL, NULL, 0);    

  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    printf("No data retrieved\n");        
    PQclear(res);
    do_exit();
  }   
    
  int rows = PQntuples(res);

  PQclear(res);

  return rows > 0;
}

int login_user(char *username, char *password) {
  char *query = "select * from chat_user where username=$1;";
  const char *param_values[1];
  param_values[0] = username;
  PGresult *res = PQexecParams(conn, query, 1, NULL, param_values, NULL, NULL, 0);    

  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    printf("No data retrieved\n");        
    PQclear(res);
    do_exit();
  }   
    
  int rows = PQntuples(res);
  if (rows == 0) 
    return -1;
  
  char *pass_from_db = PQgetvalue(res, 0, 3);

  return strcmp(password, pass_from_db) == 0;
}

void add_user(char *username, char *password, int chat_id) {
  char *query = "insert into chat_user values($1, $2, $3);";
  const char *param_values[3];
  char chid[20]; 
  sprintf(chid, "%d", chat_id); 

  param_values[0] = chid;
  param_values[1] = username;
  param_values[2] = password;
  PGresult *res = PQexecParams(conn, query, 1, NULL, param_values, NULL, NULL, 0);    

  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    PQclear(res);
    do_exit();   
  }
  PQclear(res);
}


void connect_to_db() {
  PGconn *conn = PQconnectdb("user=rajik dbname=chat");
  if (PQstatus(conn) == CONNECTION_BAD)
    do_exit();
}
