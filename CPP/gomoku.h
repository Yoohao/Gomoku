#include <mutex>
#define SIZE 10
#define TIME 300
#define BLACK 'B'
#define TURN_B false
#define WHITE 'W'
#define TURN_W true
#define BOARD '+'
using namespace std;

struct pos
{
  int x, y;
};

bool pick, TO, Exit;
int r_time;

class Gomoku
{
private:
  char map[SIZE][SIZE], winner;
  bool who; // false: black, true: white
  int remain;
  struct pos cur;
public:
  Gomoku();
  //~Gomoku();
  void GotoXY();
  bool GetWho();
  char GetWinner();
  void SetWho(bool b);
  void SetPick(bool b);
  void SetWinner(char c);
  void Control();
  void Referee();
};

void DrawBoard();
void Timer();
void Showinit(Gomoku &Game);
