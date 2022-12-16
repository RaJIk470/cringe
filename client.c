#include "lib/inet_wrap.h"
#include "lib/util.h"
#include <pthread.h>
#include <stdio.h>
#include <ncurses.h>
#include <form.h>

#define BUFF_SIZE 1024

typedef struct {
  SAI serv_addr;
  int socketfd;
} Data;

void read_from_server(void *arg);
void read_user_input(void *arg);
int check_args(int argc, char *argv[]);


WINDOW *create_win(int y, int x, int starty, int startx, int border);
void destroy_win(WINDOW *win);
void initwin();

WINDOW *output_win;
WINDOW *input_win;
FIELD *input_field[2];
FORM *form;

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

  initwin();

  pthread_create(&pid, NULL, (void *)read_from_server, (void *)&data);
  pthread_create(&pid, NULL, (void *)read_user_input, (void *)&socketfd);

  pthread_join(pid, NULL);

  _close(socketfd);

  endwin();
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
  char buff[BUFF_SIZE + 1];

  
  while ((n = read(socketfd, buff, BUFF_SIZE)) > 0) {
    buff[n] = 0;
    wprintw(output_win, "%s", buff);
    wrefresh(output_win);
  }

  /*while ((n = read(socketfd, buff, BUFF_SIZE)) > 0) {
    buff[n] = '\0';
    printf("%s", buff);
  }*/
}

void read_user_input(void *arg) {
  int ch;
  int *socketfd = (int *)arg;
  char message[BUFF_SIZE];

  while ((ch = getch()) != KEY_F(1)) {
    switch (ch) {
    case KEY_BACKSPACE:
      form_driver(form, REQ_DEL_PREV);
      break;
    case 13:
      form_driver(form, REQ_VALIDATION);
      snprintf(message, BUFF_SIZE, "%s", field_buffer(input_field[0], 0));
      char *fmsg = trim_whitespaces(message);
      wprintw(output_win, "%s\n", fmsg);
      wrefresh(output_win);
      if (strcmp(fmsg, "/exit") != 0)
        if (strnlen(fmsg, BUFF_SIZE) > 0)
          _write(*socketfd, fmsg, BUFF_SIZE);
      set_field_buffer(input_field[0], 0, "");
      form_driver(form, REQ_VALIDATION);
      break;
    default:
      form_driver(form, ch);
      break;
    }
  }

  /*while (fgets(message, BUFF_SIZE, stdin) > 0) {
    _write(*socketfd, message, BUFF_SIZE);
    if (starts_with("/exit", message))
      break;
  }*/
}

WINDOW *create_win(int y, int x, int starty, int startx, int border) {
  WINDOW *win;
  win = newwin(y, x, starty, startx);
  if (border != -1)
    box(win, border, border);
  wrefresh(win);
  return win;
}

void destroy_win(WINDOW *win) {
  wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(win);
  delwin(win);
}

void initwin() {
  int startx, starty, x, y;
  initscr();
  noecho();
  raw();
  nonl();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);

  getmaxyx(stdscr, y, x);
  y -= 3;
  startx = 0;
  starty = 0;

  refresh();
  output_win = create_win(y, x, starty, startx, -1);
  starty = y;
  y = 3;
  input_win = create_win(y, x, starty, startx, 0);

  input_field[0] = new_field(1, x - 2, starty + 1, startx + 1, 0, 0);
  input_field[1] = NULL;
  set_field_back(input_field[0], A_UNDERLINE);
  field_opts_off(input_field[0], O_AUTOSKIP);

  form = new_form(input_field);
  post_form(form);
  refresh();

  scrollok(output_win, TRUE);
  wrefresh(output_win);
  wrefresh(input_win);
}
