/*** includes ***/
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)

/*** terminal ***/
void die(const char *s){
  write(STDOUT_FILENO,"\x1b[2J",4);
  write(STDOUT_FILENO,"\x1b[H",3);
  perror(s);
  exit(1);
}

/*** data ***/
struct editorConfig{
  struct termios orig_termios;
  int screen_rows;
  int screen_columns;

};

struct editorConfig E;

/*** input ***/
char editorReadKey(){
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
  {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

/*** Output ***/

void editorDrawRows(){
  for (int i = 0; i < E.screen_rows; ++i)
  {
    write(STDOUT_FILENO,"~\r\n",3);
  }
}

/*** terminal ***/

void disableRawMode() {
  E.orig_termios.c_lflag |= ECHO;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
      die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
      die("tcgetattr");

  atexit(disableRawMode);

  struct termios raw = E.orig_termios;
  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0 ;
  raw.c_cc[VTIME] = 1 ;


  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
      die("tcsetattr");

}

void editorProcessKeypress(){
  char c = editorReadKey();

  switch (c){
    case CTRL_KEY('q'):
      write(STDOUT_FILENO,"\x1b[2J",4);
      write(STDOUT_FILENO,"\x1b[H",3);
      exit(1);
      break;
  }
}
void editorRefreshScreen(){
  write(STDOUT_FILENO,"\x1b[2J",4);
  write(STDOUT_FILENO,"\x1b[H",3);

  editorDrawRows();

  write(STDOUT_FILENO,"\x1b[H",3);

}

int getWindowSize( int *rows , int * cols){
  struct winsize ws;

  if (ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws) == -1 || ws.ws_col == 0)
  {
    return -1;
  }
  else{
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}


/*** init ***/
void initEditor(){
  if (getWindowSize(&E.screen_rows,&E.screen_columns) == -1) die("getWindowSize");
}

int main() {
  enableRawMode();
  initEditor();
  while (1){
    editorRefreshScreen();
    editorProcessKeypress();
  }
  disableRawMode();
  return 0;
}