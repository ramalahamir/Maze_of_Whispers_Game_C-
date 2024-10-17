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

struct coordinates
{
    int row;
    int col;

    coordinates() {}
    coordinates(int x, int y)
    {
        row = x;
        col = y;
    }
};

// making Node and list library for stack
struct Node
{
    coordinates cor;
    int data;
    Node *next;

    Node(int r, int c, int val)
    {
        cor.row = r;
        cor.col = c;
        data = val;
        next = nullptr;
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

class UndoStack
{
  public:
    Node *top;
    int size;
    int capacity;

    UndoStack(int undoMoves)
    {
        top = nullptr;
        capacity = undoMoves;
        size = 0;
    }

    void Push(int data, int row, int col)
    {
        Node *temp = new Node(row, col, data);

        // if capacity full
        if (size == capacity)
            return;

        if (isEmpty())
            top = temp;
        else
        {
            temp->next = top;
            top = temp;
        }
        size++;
    }

    coordinates Pop()
    {
        if (isEmpty())
            return coordinates(-1, -1);

        coordinates pop = top->cor;

        Node *temp = top;
        top = top->next;
        delete temp;

        // size will NOT decrement because undo moves are fixed
        // once used the node cannot be overwritten!

        return pop;
    }

    coordinates peek(int n = 0)
    {
        if (isEmpty())
            return coordinates(-1, -1);

        // peek into a specific level
        Node *temp = top;
        for (int i = 0; i < n; i++)
        {
            temp = temp->next;
        }
        return temp->cor;
    }

    bool isEmpty() { return top == nullptr; }
    bool isFull() { return size == capacity; }
};

class Player
{
  public:
    // counter
    int move_no;
    int undoMove_no;
    // limits
    int total_moves;
    int total_undo;

    int X;
    int Y;

    Player() { move_no = 0; }

    // parameters:
    // Xcor is sent to adjust the position of prompt display
    // previous position of player

    void movement(int grid_Xcor, UndoStack *&stack)
    {
        mvprintw(grid_Xcor + 2, 30,
                 "enter movement : (using up, down, left, right arrowkeys  or "
                 " press u for undo or "
                 " press esc to quit): ");
        int input;
        bool polarOpposite = false;

        // temprorary storage for new val
        int new_x;
        int new_y;

        // get the current move
        coordinates cor = stack->peek();

        // get the move before the current (top)
        coordinates prev_cor = stack->peek(1);

        switch (input)
        {
            case KEY_UP:
                new_x = cor.row - 1;
                new_y = cor.col;
                break;

            case KEY_DOWN:
                new_x = cor.row + 1;
                new_y = cor.col;
                break;

            case KEY_LEFT:
                new_x = cor.row;
                new_y = cor.col - 1;
                break;

            case KEY_RIGHT:
                new_x = cor.row;
                new_y = cor.col + 1;
                break;

            // esc key pressed
            case 27:
                // exit the game
                endwin();
                break;

            default:
                mvprintw(grid_Xcor + 3, 50, "invalid move!");
                break;
        }

        // check if new move isn't polar opposite to the current move
        if (new_x == prev_cor.row && new_y == prev_cor.col)
        {
            mvprintw(grid_Xcor + 3, 50, "invalid move!");
            polarOpposite = true;
        }
        else
        {
            X = new_x;
            Y = new_y;
        }

        // restriction on sudden pack paddle movement handling
        if (polarOpposite)
        {
            mvprintw(grid_Xcor + 4, 50, "you cannot move back unless its undo");
            mvprintw(grid_Xcor + 4, 50, "do you want it to be undo? (y/n):");
            char ch = getch();
            if (ch == 'y' || 'Y')
                input = 'U';
        }

        // undo movement
        if (input == 'u' || input == 'U')
        {

            // popping the current move
            coordinates prev_move = stack->Pop();

            // now pop the last move (used for undo)
            prev_move = stack->Pop();

            // new move will be the previous move
            X = prev_move.row;
            Y = prev_move.col;
        }
    }
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

    // keeps track of the players old position
    GridCell *player_prevPos;
    UndoStack *stack;

    int seed; // for the random function

    // starting grid X cor and Ycor on screen
    // this will be stored for display setting purposes
    int grid_Xcor, grid_Ycor;

    Grid(int lvl)
    {
        level = lvl;
        score = 0;
        player = new Player();
        player_prevPos = nullptr;

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

        // generating seed
        seed = 5678 / dimension;

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

        // intitalizing stack size
        stack = new UndoStack(undoMoves);

        head = nullptr;
        tail = nullptr;
    }

    void setting_distance_differences()
    {
        player_key_dist =
            cityBlockDistance(player->X, player->Y, key.key_x, key.key_y);
        player_door_dist =
            cityBlockDistance(player->X, player->Y, door.door_x, door.door_y);
        key_door_dist =
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

                // setting the player on the grid
                else if (i == player->X && j == player->Y)
                {
                    cell = new GridCell(i, j, 'P');
                    player_prevPos = cell;
                    stack->Push(player_prevPos->data, player_prevPos->row,
                                player_prevPos->col);
                }

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

    // overwrites the grid cells to show updated position
    void adjustingPlayer_onGrid()
    {
        // previous osition is filled with .
        player_prevPos->data = '.';

        // if player moved up
        if (player->X == player_prevPos->row - 1)
            player_prevPos->up->data = 'P';

        // if player moved down
        else if (player->X == player_prevPos->row + 1)
            player_prevPos->down->data = 'P';

        // if player moved left
        else if (player->Y == player_prevPos->col - 1)
            player_prevPos->left->data = 'P';

        // if player moved right
        else if (player->Y == player_prevPos->col + 1)
            player_prevPos->right->data = 'P';

        // no change in position
        else
            player_prevPos->data = 'P';

        // update the players new posiion as prev
        player_prevPos->row = player->X;
        player_prevPos->col = player->Y;

        // pushing it into the stack
        stack->Push(player_prevPos->data, player_prevPos->row,
                    player_prevPos->col);
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

        // player's new position updation
        adjustingPlayer_onGrid();

        GridCell *row = head;
        grid_Xcor = 7;
        while (row != nullptr)
        {
            grid_Ycor = 45;
            GridCell *col = row;
            while (col != nullptr)
            {
                mvprintw(grid_Xcor, grid_Ycor, "%c", col->data);
                col = col->right;
                grid_Ycor += 3; // incrementing col pos
            }
            grid_Xcor += 1; // incremeting row pos
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
        int a = 1122;
        int c = 12345;
        // using LCG formula
        seed = (a * seed + c) % dimension;
        return seed;
    }
};

int main()
{
    initscr(); // Initialize the screen

    // for taking keyboard inputs
    keypad(stdscr,
           TRUE); // for capturing special keys like arrow keys
    noecho();     // Don't echo the input

    Grid G(1);
    G.makGrid();
    G.displayGrid();

    refresh(); // Refreshes the screen to see the updates
    getch();
    endwin();
    // endwin() ends the "curses mode" and brings the terminal back to normal
    return 0;
}