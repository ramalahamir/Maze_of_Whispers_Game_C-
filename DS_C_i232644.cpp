#include "C:/Users/Administrator/vcpkg/installed/x64-windows/include/curses.h"
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
    char symb = 'K';
    bool status = false;
};

struct Door
{
    int door_x;
    int door_y;
    char symb = 'D';
};

struct Coin
{
    int coin_x;
    int coin_y;
    char symb = 'C';
};

struct Bomb
{
    int bomb_x;
    int bomb_y;
    bool detonate;
    char symb = 'B';
};

class Player
{
  public:
    int move_no;
    int undoMove_no;
    int total_moves;
    int total_undo;

    int X;
    int Y;
};

class Grid
{
  public:
    GridCell *head;
    GridCell *tail;

    int level;
    int dimension;
    int moves;
    int undoMoves;
    int score;

    Key key;
    Player *player;
    Door door;

    Grid(int lvl)
    {
        level = lvl;
        switch (level)
        {
            case 1:
                dimension = 5;
                break;
            case 2:
                dimension = 10;
                break;
            case 3:
                dimension = 15;
                break;
        }
        head = nullptr;
        tail = nullptr;
    }

    void makGrid()
    {
        GridCell *rowHead = nullptr;
        GridCell *cell;
        GridCell *prevRowHead;

        int rows, cols;
        rows = cols = dimension + 2; // because of boundaries

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                // setting the up and down boundaries
                if (i == 0 || j == 0 || i == rows - 1 || j == cols - 1)
                    cell = new GridCell(i, j, '#');
                // regular cell
                else
                    cell = new GridCell(i, j, '.');

                // first node
                if (head == nullptr)
                {
                    head = cell;
                    tail = cell;
                }

                if (j == 0)
                {
                    prevRowHead = cell;
                    tail = cell;
                }

                // setting links for the cell other than the first one

                // linking the upper adjacent cell if its not null
                if (rowHead != nullptr)
                {
                    // itearating the upper adjaecent row to find the upper
                    // adjacent cell
                    GridCell *temp = rowHead;
                    while (temp->right != nullptr && temp->col != cell->col)
                    {
                        temp = temp->right;
                    }
                    cell->up = temp;
                    temp->down = cell;
                }

                // linking the left adjacent cell if its not a boundary
                if (tail != nullptr)
                {
                    cell->left = tail;
                    tail->right = cell;
                }

                // updating the tail to the new cell
                // so that the boundary nodes point to null
                if (j != cols - 1)
                    tail = cell;
            }
            rowHead = prevRowHead;
        }
    }

    void displayGrid()
    {
        cout << "\n\t\tLEVEL: " << level;
        // cout << "\n\n\tRemaining Moves: " << moves - player->move_no
        //      << "\tRemaining Undo Moves: " << undoMoves -
        //      player->undoMove_no;
        // cout << "\n\tScore: " << score << "\tkey status: " << key.status;
        // hintSystem();

        GridCell *row = head;
        while (row != nullptr)
        {
            cout << "\n\t\t";
            GridCell *col = row;
            while (col != nullptr)
            {
                cout << col->data << "\t";
                col = col->right;
            }
            cout << endl << endl;
            row = row->down;
        }
    }

    int cityBlockDistance(int x1, int y1, int x2, int y2)
    {
        int mod_x = (x2 - x1 < 0) ? x1 - x2 : x2 - x1;
        int mod_y = (y2 - y1 < 0) ? y1 - y2 : y2 - y1;

        int dist = mod_x + mod_y;
        return dist;
    }

    void hintSystem()
    {
        int dist =
            cityBlockDistance(player->X, player->Y, key.key_x, key.key_y);
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
    Grid G(1);
    G.makGrid();
    G.displayGrid();
    return 0;
}