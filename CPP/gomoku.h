#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ncurses.h>

#define SIZE 10
#define TIME 300
#define BLACK 'B'
#define TURN_B false
#define WHITE 'W'
#define TURN_W true
#define BOARD '+'
#define MAXSIZE 1000
using namespace std;

struct pos
{
  int x, y;
};

bool pick, TO, Exit, turn;
int r_time;

class CONNECTION
{
protected:
  int sock;
  char buffer[MAXSIZE];
  struct addrinfo server;
  bool conn;
public:
    CONNECTION();
    ~CONNECTION();
    void ConnectTCP();
    virtual void Recieve() = 0;
    virtual void Send() = 0;
    int GetSock();
    bool GetConnection();
};

class Gomoku : public CONNECTION
{
private:
  char map[SIZE][SIZE], winner;
  bool who;
  bool internet;
  int remain;
  struct pos cur;
public:
  void Recieve()
  {
    memset(buffer, 0, sizeof(buffer));
    recv(this->sock, buffer, 100, 0);
    mvprintw(14, 0, "%s", buffer);
  }
  void Send()
  {
    char buff[15];
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "/gomoku %d%d\r\n", cur.x, cur.y);
    if(send(this->sock, buff, 15, 0) == -1)
      return;
  }
  Gomoku();
  void RECV_DRAW();
  void GotoXY();
  bool GetWho();
  char GetWinner();
  bool GetInternet();
  CONNECTION GetConnection();
  void SetWho(bool b);
  void SetPick(bool b);
  void SetWinner(char c);
  void SetInternet(bool yes);
  void Control();
  void Referee(struct pos CUR);
  void Draw(struct pos cur, bool who);
};

void DrawBoard(bool who);
void Timer();
void Showinit(Gomoku &Game);
