#include "gomoku.h"
#include <ctime>
#include <iostream>
#include <ncurses.h>
#include <mutex>
using namespace std;

/*---------class Gomoku---------*/
Gomoku::Gomoku ()
{

  for(int i = 0; i < 10; i++)
    for(int j = 0; j < 10; j++)
        map[i][j] = 'E';
  cur.x = cur.y = 0;
  who = pick = Exit = false;
  winner = 'E';
  remain = 100;
}

void Gomoku::GotoXY()
{
  move(cur.y+2, cur.x*2+3);
}

bool Gomoku::GetWho(){return this->who;}

char Gomoku::GetWinner(){return this->winner;}

void Gomoku::SetWho(bool b){this->who = b;}

void Gomoku::SetWinner(char c){this->winner = c;}

void Gomoku::Control()
{
  GotoXY();
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
        remain-=1;
        if(who) attron(A_REVERSE);
        mvaddch(cur.y+2, cur.x*2+3, who?WHITE:BLACK);
        if(who) attroff(A_REVERSE);
        mvprintw(8, 2*SIZE+10, "Remain: %d  ", remain);
        Referee();
        map[cur.y][cur.x]= who?WHITE:BLACK;
        SetWho(who?TURN_B:TURN_W);
        pick = true;

      }
      break;
    case 27:
      mvaddstr(SIZE+5, 0, "Exit?(Y/N) ");
      move(SIZE+5, 11);
      refresh();
      move(SIZE+5, 11);
      echo();
      char msg;
      bool check;
      check = false;
      while(!check)
      {
        msg = getch();
        getch();
        if(msg == 'N' || msg == 'n')
          break;
        else if(msg == 'Y' || msg == 'y')
        {
          Exit = true;
          break;
        }
        else
        {
          mvaddstr(SIZE+6, 0, "Invalid argument, try again.");
          mvaddstr(SIZE+5, 11,"        ");
          move(SIZE+5, 11);
          refresh();
        }
      }
      noecho();
      mvaddstr(SIZE+5, 0,"                            ");
      mvaddstr(SIZE+6, 0,"                            ");
      break;
    default:
      break;
  }
  mvprintw(7, 2*SIZE+10+9, "%c, %d", (char)cur.x+'A', cur.y);
  mvaddch(5, 2*SIZE+10+6, (char)(r_time/60+'0'));
  mvprintw(5, 2*SIZE+10+8, "%d ", r_time%60);
}

void Gomoku::Referee()
{
  int dir[8] = {0};
  char Obj = who?WHITE:BLACK;
  for(int i=1; i<5; i++)
  {
    if(cur.y-i>=0 && map[cur.y-i][cur.x]==Obj)
      dir[0]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(cur.x+i<10 && map[cur.y][cur.x+i]==Obj)
      dir[2]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(cur.y+i<10 && map[cur.y+i][cur.x]==Obj)
      dir[4]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(cur.x-i>=0 && map[cur.y][cur.x-i]==Obj)
      dir[6]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(cur.x+i<10 && cur.y-i>=0 && map[cur.y-i][cur.x+i]==Obj)
      dir[1]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(cur.x+i<10 && cur.y+i<10 && map[cur.y+i][cur.x+i]==Obj)
      dir[3]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(cur.x-i>=0 && cur.y+i<10 && map[cur.y+i][cur.x-i]==Obj)
      dir[5]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(cur.x-i>=0 && cur.y-i>=0 && map[cur.y-i][cur.x-i]==Obj)
      dir[7]++;
    else break;
  }
  for(int i=0; i<5; i++)
  {
    if((dir[i]=dir[i]+dir[i+4]) == 4)
    {
       SetWinner(Obj);
       break;
    }
  }
}


/*---------function---------*/
void DrawBoard()
{
  for(int i=0; i<SIZE; i++)
  {
    mvaddch(0, 2*(i+1)+1, (char)i+'A');
    mvaddch(i+2, 0, (char)i+'0');
    mvaddch(i+2, 1, '|');
    mvaddch(i+2, 2*SIZE+3, '|');
    mvaddch(1, 2*(i+1)+1, '-');
    mvaddch(SIZE+2, 2*(i+1)+1, '-');
  }
  for(int i = 0; i < SIZE; i++)
    for(int j = 0; j < SIZE; j++)
      mvaddch(i+2, 2*j+3, BOARD);
  mvaddstr(4, 2*SIZE+10, "Now Return:");
  mvaddstr(5, 2*SIZE+10, "Time:  :");
  mvaddstr(7, 2*SIZE+10, "Now pos: A, 0");
  mvprintw(8, 2*SIZE+10, "Remain: %d", SIZE*SIZE);
  mvaddstr(10, 2*SIZE+10, "Winner: None");
  refresh();
}

void Timer ()
{
  int t = TIME;
  while(t--)
  {
    r_time = t;
    clock_t end;
    end = clock () + CLOCKS_PER_SEC ;
    while (clock()<end) if(pick || Exit)  return;
  }
  TO = true;
  pick = true;
  return;
}

void Showinit(Gomoku &Game)
{
  if(Game.GetWho()) attron(A_REVERSE);
  mvaddstr(4, 2*SIZE+10+12, Game.GetWho()?"White":"Black");
  if(Game.GetWho()) attroff(A_REVERSE);
  mvaddch(5, 2*SIZE+10+6, (char)(TIME/60)+'0');
  mvprintw(5, 2*SIZE+10+8, "%d", TIME%60);
}
