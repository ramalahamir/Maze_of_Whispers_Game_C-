// #include
// "C:\\Users\\Administrator\\OneDrive\\Desktop\\university\\dataStructures\\PDCurses_library\\curses.h"
#include <iostream>

using namespace std;

class GridCell
{
  public:
    int row;
    int col;
    char data;

    GridCell *up;
    GridCell *down;
    GridCell *left;
    GridCell *right;

    GridCell(int r, int c, char val)
    {
        data = val;
        row = r;
        col = c;
        up = down = left = right = nullptr;
    }
};

struct Key
{
    int key_x;
    int key_y;
};

struct Door
{
    int door_x;
    int door_y;
};

struct Coin
{
    int coin_x;
    int coin_y;
};

struct Bomb
{
    int bomb_x;
    int bomb_y;
    bool detonate;
};

class Grid
{
  public:
    GridCell *head;
    int level;
    int dimension;
    int moves;
    int undoMoves;
    Key key;

    Grid(int lvl)
    {
        level = lvl;
        switch (level)
        {
            case 1:
                dimension = 5;
                // moves = cityBlockDistance(key_x, player_x, )
            case 2:
                dimension = 10;
            case 3:
                dimension = 15;
        }
        head = nullptr;
    }

    int cityBlockDistance(int x1, int y1, int x2, int y2)
    {
        int mod_x = (x2 - x1 < 0) ? x1 - x2 : x2 - x1;
        int mod_y = (y2 - y1 < 0) ? y1 - y2 : y2 - y1;

        int dist = mod_x + mod_y;
        return dist;
    }
};

int main()
{
    // Initialize PDCurses
    // initscr();               // Start curses mode
    // printw("Hello, World!"); // Print message to screen
    // refresh();               // Refresh to show the message
    // getch();                 // Wait for user input
    // endwin();                // End curses mode

    // int *x = new int();
    // int sum = 2 + static_cast<int>(*x);
    // cout << x;

    return 0;
}