#include "gomoku.h"
#include <ctime>
#define IP "192.168.1.108"
#define PORT "6666"
using namespace std;

/*---------class Gomoku---------*/
Gomoku::Gomoku ()
{
  for(int i = 0; i < 10; i++)
    for(int j = 0; j < 10; j++)
        map[i][j] = 'E';
  cur.x = cur.y = 0;
  who = pick = Exit = internet = false;
  winner = 'E';
  remain = 100;
}

void Gomoku::RECV_DRAW()
{
  this->Recieve();
  struct pos tmp;
  tmp.x = (int)buffer[0]-'0';
  tmp.y = (int)buffer[1]-'0';
  Draw(tmp, !this->who);
}

void Gomoku::GotoXY()
{
  move(cur.y+2, cur.x*2+3);
}

bool Gomoku::GetWho(){return this->who;}

char Gomoku::GetWinner(){return this->winner;}

bool Gomoku::GetInternet(){return this->internet;}

void Gomoku::SetWho(bool b){this->who = b;}

void Gomoku::SetWinner(char c){this->winner = c;}

void Gomoku::SetInternet(bool yes)
{
  this->internet = yes;
  if(yes)
  {
    ConnectTCP();
    this->Recieve();
    if(buffer[0] == 0x00)
      who = false;
    else if (buffer[0] == 0x01)
      who = true;
  }
  else
    return;
}

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
    case ' ':
      if (map[cur.y][cur.x] == 'E')
      {
        this->Draw(cur, who);
        this->Send();
      }
      break;
    case 'Q':
    case 'q':
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
  mvprintw(7, 2*SIZE+10+9, "%c, %d", (char)cur.x+'0', cur.y);
  mvaddch(5, 2*SIZE+10+6, (char)(r_time/60+'0'));
  mvprintw(5, 2*SIZE+10+8, "%c%c", (char)(r_time%60/10+'0'), (char)(r_time%60%10+'0'));
}

void Gomoku::Referee(struct pos CUR)
{
  int dir[8] = {0};
  //mvprintw(14, 0, "%d", turn?1:0);
  char Obj = turn?WHITE:BLACK;
  for(int i=1; i<5; i++)
  {
    if(CUR.y-i>=0 && map[CUR.y-i][CUR.x]==Obj)
      dir[0]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(CUR.x+i<10 && map[CUR.y][CUR.x+i]==Obj)
      dir[2]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(CUR.y+i<10 && map[CUR.y+i][CUR.x]==Obj)
      dir[4]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(CUR.x-i>=0 && map[CUR.y][CUR.x-i]==Obj)
      dir[6]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(CUR.x+i<10 && CUR.y-i>=0 && map[CUR.y-i][CUR.x+i]==Obj)
      dir[1]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(CUR.x+i<10 && CUR.y+i<10 && map[CUR.y+i][CUR.x+i]==Obj)
      dir[3]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(CUR.x-i>=0 && CUR.y+i<10 && map[CUR.y+i][CUR.x-i]==Obj)
      dir[5]++;
    else break;
  }
  for(int i=1; i<5; i++)
  {
    if(CUR.x-i>=0 && CUR.y-i>=0 && map[CUR.y-i][CUR.x-i]==Obj)
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

void Gomoku::Draw(struct pos POS, bool WHO)
{
    this->remain-=1;
    if(WHO) attron(A_REVERSE);
    mvaddch(POS.y+2, POS.x*2+3, WHO?WHITE:BLACK);
    if(WHO) attroff(A_REVERSE);
    mvprintw(8, 2*SIZE+10, "Remain: %d  ", this->remain);
    map[POS.y][POS.x]= WHO?WHITE:BLACK;
    Referee(POS);
    turn = !turn;
    pick = true;
}

/*---------class CONNECTION---------*/
CONNECTION::CONNECTION()
{
  conn = false;
  memset(&server, 0, sizeof(server));
}

CONNECTION::~CONNECTION()
{
  close(sock);
}

void CONNECTION::ConnectTCP()
{
  struct addrinfo *info, *p;
  server.ai_family = AF_UNSPEC;
  server.ai_socktype = SOCK_STREAM;
  getaddrinfo(IP, PORT, &server, &info);
  for (p = info; p!=NULL; p=p->ai_next)
  {
    if((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
      continue;
    if(connect(sock, p->ai_addr, p->ai_addrlen) == -1)
    {
       close(sock);
       continue;
    }
    conn = true;
    printf("connect!!\n");
    break;
  }
  if(p==NULL)
    printf("NO!\n");
  freeaddrinfo(info);
  int yes=1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
}

int CONNECTION::GetSock()
{
  return this->sock;
}

bool CONNECTION::GetConnection()
{
  return this->conn;
}
/*---------function---------*/
void DrawBoard(bool who)
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
  mvaddstr(3, 2*SIZE+10, "You are: ");
  if(who) attron(A_REVERSE);
  mvprintw(3, 2*SIZE+10+9, "%s", who?"White":"Black");
  if(who) attroff(A_REVERSE);
  mvaddstr(4, 2*SIZE+10, "Now turn to:");
  mvaddstr(5, 2*SIZE+10, "Time: 5:00");
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
  if(turn) attron(A_REVERSE);
  mvaddstr(4, 2*SIZE+10+12, turn?"White":"Black");
  if(turn) attroff(A_REVERSE);
  mvaddch(5, 2*SIZE+10+6, (char)(TIME/60)+'0');
  mvprintw(5, 2*SIZE+10+8, "%d%d", TIME%60/10, TIME%60%10);
}
