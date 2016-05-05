#include <ncurses.h>
#include <thread>
#include "gomoku.cpp"
#define SIZE 10

Gomoku Game;

int main()
{
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, true);
  DrawBoard();
  refresh();
  while(1)
  {
    Showinit(Game);
    refresh();
    pick = false;
    TO = false;
    thread t(Timer);
    while(!pick && !TO && !Exit)
      Game.Control();
    t.join();
    if(Exit)
    {
      break;
    }
    if(TO) Game.SetWinner(Game.GetWho()?BLACK:WHITE);
    if(Game.GetWinner()!='E')
    {
      if(Game.GetWinner() == WHITE) attron(A_REVERSE);
      mvaddstr(10, 2*SIZE+10+8, Game.GetWinner()==WHITE?"White":"Black");
      if(Game.GetWinner() == WHITE) attroff(A_REVERSE);
      getch();
      break;
    }
    flash();
  }
  erase();
  mvaddstr(0, 0, "Good bye!");
  refresh();
  getch();
  endwin();
  return 0;
}
