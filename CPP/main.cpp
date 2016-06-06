#include <thread>
#include "gomoku.cpp"
#define SIZE 10

void EnableCurses(bool yes=true)
{
  if(yes)
  {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, true);
    nodelay(stdscr, true);
  }
}

Gomoku Game;
int main()
{
  Game.SetInternet(true);
  cout<<"waiting..."<<endl;
  cout<<"press any key to start game"<<endl;
  getchar();

  EnableCurses();
  DrawBoard(Game.GetWho());
  refresh();
  turn = false;
  while(1)
  {
    Showinit(Game);
    refresh();
    if((turn != Game.GetWho()) && Game.GetInternet())
    {
      mvaddstr(12, 2*SIZE+10, "Status: waiting...");
      refresh();
      Game.RECV_DRAW();
    }
    refresh();
    if(Game.GetWinner()!='E')
    {
      if(Game.GetWinner() == WHITE) attron(A_REVERSE);
      mvaddstr(10, 2*SIZE+10+8, Game.GetWinner()==WHITE?"White":"Black");
      if(Game.GetWinner() == WHITE) attroff(A_REVERSE);
      nodelay(stdscr, false);
      getch();
      break;
    }
    mvaddstr(12, 2*SIZE+10, "Status: thinking..");
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
      nodelay(stdscr, false);
      getch();
      break;
    }

    //flash();
  }
  erase();
  mvaddstr(0, 0, "Good bye!");
  refresh();
  getch();
  endwin();

  return 0;
}
