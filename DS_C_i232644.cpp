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

    Player() { move_no = undoMove_no = 0; }
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

    int player_key_dist;
    int player_door_dist;
    int key_door_dist;

    Key key;
    Player *player;
    Door door;

    int seed; // for the random function

    Grid(int lvl)
    {
        seed = 123;
        level = lvl;
        player = new Player();

        // setting the dimension
        switch (level)
        {
            case 1:
                dimension = 10;
                break;
            case 2:
                dimension = 15;
                break;
            case 3:
                dimension = 12;
                break;
        }

        // setting the random positions for player, key and door
        key.key_x = random();
        key.key_y = random();
        player->X = random();
        player->Y = random();
        door.door_x = random();
        door.door_y = random();

        // calculating distance differences for player, key and door
        setting_distance_differences();
        int dist = player_door_dist + player_key_dist + key_door_dist;

        // setting the moves
        switch (level)
        {
            case 1:
                moves = dist + 6;
                undoMoves = 6;
                break;
            case 2:
                moves = dist + 2;
                undoMoves = 4;
                break;
            case 3:
                moves = dist;
                undoMoves = 1;
                break;
        }

        // setting for player object
        player->total_moves = moves;
        player->total_undo = undoMoves;

        head = nullptr;
        tail = nullptr;
    }

    void setting_distance_differences()
    {
        player_key_dist =
            cityBlockDistance(player->X, player->Y, key.key_x, key.key_y);
        player_door_dist =
            cityBlockDistance(player->X, player->Y, door.door_x, door.door_y);
        int key_door_dist =
            cityBlockDistance(key.key_x, key.key_y, door.door_x, door.door_y);
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
        clear();

        mvprintw(1, 60, "LEVEL: %d", level);
        mvprintw(2, 30, "Remaining Moves: %d", moves - player->move_no);
        mvprintw(2, 80, "Remaining Undo Moves: %d",
                 undoMoves - player->undoMove_no);
        mvprintw(3, 30, "Score: %d", score);
        mvprintw(3, 80, "key status:  %d", key.status);
        hintSystem();

        GridCell *row = head;
        int x = 7;
        while (row != nullptr)
        {
            int y = 45;
            GridCell *col = row;
            while (col != nullptr)
            {
                mvprintw(x, y, "%c", col->data);
                col = col->right;
                y += 3; // incrementing col pos
            }
            x += 1; // incremeting row pos
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
        mvprintw(5, 30, "HINT: ");
        if (player_key_dist <= 3)
            mvprintw(5, 50, "Getting Closer");
        else
            mvprintw(5, 50, "Further Away");
    }

    int random()
    {
        int a = 11223344;
        int c = 12345;
        // using LCG formula
        seed = (a * seed + c) % dimension;
        return seed;
    }
};

int main()
{
    initscr(); // Initialize the screen

    Grid G(1);
    G.makGrid();
    G.displayGrid();

    refresh(); // Refreshes the screen to see the updates
    getch();
    endwin();
    // endwin() ends the "curses mode" and brings the terminal back to normal
    return 0;
}