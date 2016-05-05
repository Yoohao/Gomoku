#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <ctime>
#define SIZE 10
#define TIME 3
#define BLACK "O"
#define WHITE "O"
#define BOARD "+"
using namespace std;

struct pos
{
  int x, y;
};

int c=SIZE*SIZE;
char map[SIZE][SIZE];
struct pos cur;
bool black, ctrl, winner;
mutex m;

void gotoxy(struct pos &cur)
{
  move(cur.y+2, cur.x*2+3);
}

void showpos(struct pos &cur)
{
  mvaddch(7, 2*SIZE+10+9, (char)cur.x+'0');
  mvaddch(7, 2*SIZE+10+10, ',');
  mvaddch(7, 2*SIZE+10+12, (char)cur.y+'0');
}

void showtime(int  &t)
{
  mvaddch(5, 2*SIZE+10+6, (char)t/10+'0');
  mvaddch(5, 2*SIZE+10+7, (char)t%10+'0');
}

void control()
{
  gotoxy(cur);
  refresh();
  switch (getch())
  {
    case KEY_UP:
    case 'W':
    case 'w':
      if(cur.y > 0)
        cur.y -= 1;
      else cur.y = SIZE-1;
      break;
    case KEY_DOWN:
    case 'S':
    case 's':
      if(cur.y+1 < SIZE)
        cur.y += 1;
      else cur.y = 0;
      break;
    case KEY_RIGHT:
    case 'D':
    case 'd':
      if(cur.x+1 < SIZE)
        cur.x += 1;
      else cur.x = 0;
      break;
    case KEY_LEFT:
    case 'A':
    case 'a':
      if(cur.x > 0)
        cur.x -= 1;
      else cur.x = SIZE-1;
      break;
    case 'C':
    case 'c':
      break;
    case 10:
      if (map[cur.y][cur.x] == 'E')
      {
        c--;
        if(!black) attron(A_REVERSE);
        mvaddstr(cur.y+2, cur.x*2+3, black?BLACK:WHITE);
        if(!black) attroff(A_REVERSE);
        mvaddch(8, 2*SIZE+10+8, (char)c/100+'0');
        mvaddch(8, 2*SIZE+10+9, (char)c%100/10+'0');
        mvaddch(8, 2*SIZE+10+10, (char)c%10+'0');
        map[cur.y][cur.x]= black?'B':'W';
        black = !black;
        ctrl = true;
      }
      break;
    default:
      break;
  };
  mvaddch(5, 2*SIZE+10+6, (char)(r_time/60+'0'));
  mvaddch(5, 2*SIZE+10+8, (char)(r_time%60/10+'0'));
  mvaddch(5, 2*SIZE+10+9, (char)(r_time%60%10+'0'));
  showpos(cur);
  gotoxy(cur);
}

void timer()
{
  int t=TIME;
  while(t--)
  {
    r_time = t;
    if (ctrl) return;
    m.lock();
    showtime(t);
    gotoxy(cur);
    m.unlock();
    clock_t endwait;
    endwait = clock () + CLOCKS_PER_SEC ;
    while (clock() < endwait)
      if(ctrl) return;
  }
    black = !black;
    ctrl = true;
    return;
}



void referee()
{
  char Obj = black?'W':'B';
  int dir[8] = {0};
  for(int i = 1; i<=4; i++)
  {
    if (cur.y-i>=0 && map[cur.x][cur.y-i]==Obj) //up
      dir[0]+=1;
    if (cur.x+i<10 && map[cur.x+i][cur.y]==Obj) //right
      dir[2]+=1;
    if (cur.x-i>=0 && map[cur.x-i][cur.y]==Obj) //
      dir[6]+=1;
    if (cur.y+i<10 && map[cur.x][cur.y+i]==Obj)
      dir[4]+=1;
    if (cur.y-i>=0 && cur.x+i<10 && map[cur.x+i][cur.y-i]==Obj)
      dir[1]+=1;
    if (cur.y+i<10 && cur.x+i<10 && map[cur.x+i][cur.y+i]==Obj)
      dir[3]+=1;
    if (cur.y+i<10 && cur.x-i>=0 && map[cur.x-i][cur.y+i]==Obj)
      dir[5]+=1;
    if (cur.y-i>=0 && cur.x-i>=10 && map[cur.x-i][cur.y-i]==Obj)
      dir[7]+=1;
  }
}

int main()
{
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, true);

  black = true;

  for (int i = 0; i<SIZE; i++)
    for (int j = 0; j<SIZE; j++)
      map[i][j] = 'E';
  mvaddstr(0, 0, "   0 1 2 3 4 5 6 7 8 9");
  mvaddstr(1, 0, "   - - - - - - - - - -");
  for(int i=0; i<SIZE; i++)
  {
    mvaddch(i+2, 0, char(i+'0'));
    mvaddstr(i+2, 1, "|");
    mvaddstr(i+2, 2*SIZE+3, "|");
    for(int j=0; j<SIZE; j++)
      mvaddstr(i+2, 2*j+3, BOARD);
  }
  mvaddstr(SIZE+2, 0, "   - - - - - - - - - -");
  cur.x = 0;
  cur.y = 0;
  mvaddstr(4, 2*SIZE+10, "Now Return:");
  mvaddstr(5, 2*SIZE+10, "Time:");
  mvaddstr(5, 2*SIZE+10+8, " s");
  mvaddstr(7, 2*SIZE+10, "Now pos:");
  mvaddstr(8, 2*SIZE+10, "Remain:");
  mvaddch(8, 2*SIZE+10+8, (char)c/100+'0');
  mvaddch(8, 2*SIZE+10+9, (char)c%100/10+'0');
  mvaddch(8, 2*SIZE+10+10, (char)c%10+'0');
  refresh();

  while(1)
  {
    ctrl = false;
    if(!black) attron(A_REVERSE);
    mvaddstr(4, 2*SIZE+10+12, black?"Black":"White");
    if(!black) attroff(A_REVERSE);
    thread t(timer);
    while(!ctrl)
      control();
    t.join();
    flash();
  }
  getch();
  endwin();
}
