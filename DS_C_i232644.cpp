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
    int size;
    int total_undo;

  public:
    Node *top;

    UndoStack(int undoMoves)
    {
        top = nullptr;
        size = 0;               // counter
        total_undo = undoMoves; // specific number of popping allowed!
    }

    void Push(int data, int row, int col)
    {
        Node *temp = new Node(row, col, data);

        if (isEmpty())
            top = temp;
        else
        {
            temp->next = top;
            top = temp;
        }
    }

    coordinates Pop()
    {
        if (isEmpty() || size == total_undo)
            return coordinates(-1, -1);

        coordinates pop = top->cor;

        Node *temp = top;
        top = top->next;
        delete temp;

        size++; // counter for no of undomoves
        return pop;
    }

    coordinates peek(int n = 0)
    {
        if (isEmpty())
            return coordinates(-1, -1);

        // peek into a specific level
        Node *temp = top;
        for (int i = 0; i <= n; i++)
        {
            if (temp->next == nullptr)
                break;
            temp = temp->next;
        }
        return temp->cor;
    }

    bool isEmpty() { return top == nullptr; }
    int undoMovesLeft() { return size; }

    // displaying stack
    void display()
    {
        if (isEmpty())
        {
            mvprintw(1, 100, "Stack is empty.");
            refresh(); // Refresh the screen to show changes
            return;
        }

        mvprintw(1, 100, "player move history (x, y):");
        refresh(); // Refresh after printing header

        Node *temp = top;
        int line = 2; // Start displaying from row 2

        while (temp != nullptr)
        {
            // Print the row and col of each node in the stack
            mvprintw(line, 105, "(%d, %d)", temp->cor.row, temp->cor.col);
            line++; // Move to the next line for the next node
            temp = temp->next;
        }

        refresh(); // Refresh to update the screen with the final output
    }
};

class Player
{
  public:
    // counter
    int move_no;

    // player coordinates
    int X;
    int Y;

    Player() { move_no = 0; }
    void increment_move_no() { move_no++; }
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
    int remaining_moves;

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
                dimension = 20;
                break;
        }

        // generating seed
        seed = 235 / dimension;

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
        // initially both are same
        remaining_moves = moves;

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

    void player_movement(int input)
    {
        bool polarOpposite = false;
        bool invalid_move = false;

        // temporary storage for new values
        int new_x = player->X;
        int new_y = player->Y;

        // store the direction of the last move
        static char last_move = '0'; // 'w', 's', 'a', 'd' for directions

        switch (input)
        {
            // UP
            case 'w':
            case 'W':
                if (last_move == 's') // if the last move was down
                    polarOpposite = true;
                new_x = player->X - 1;
                break;

            // DOWN
            case 's':
            case 'S':
                if (last_move == 'w') // if the last move was up
                    polarOpposite = true;
                new_x = player->X + 1;
                break;

            // LEFT
            case 'a':
            case 'A':
                if (last_move == 'd') // if the last move was right
                    polarOpposite = true;
                new_y = player->Y - 1;
                break;

            // RIGHT
            case 'd':
            case 'D':
                if (last_move == 'a') // if the last move was left
                    polarOpposite = true;
                new_y = player->Y + 1;
                break;

            // Undo movement
            case 'u':
            case 'U':
            {
                coordinates prev_move = stack->Pop();
                if (prev_move.row != -1 &&
                    prev_move.col != -1) // for the initial case
                {
                    player->X = prev_move.row;
                    player->Y = prev_move.col;
                }
                return; // return control
            }

            default:
                invalid_move = true;
                break;
        }

        // Check if move is within grid boundaries
        if (new_x == 0 || new_x == dimension + 1 || new_y == 0 ||
            new_y == dimension + 1)
            invalid_move = true;

        // If the move is polar opposite, don't allow it without undo
        if (polarOpposite)
        {
            mvprintw(grid_Xcor + 5, 20,
                     "You cannot move back without undo. Press 'u' to undo.");
            refresh();
            invalid_move = true;
        }

        if (invalid_move)
        {
            mvprintw(grid_Xcor + 5, 20, "Invalid move!");
            refresh();
            return; // again return
        }

        // Update player position if valid move
        player->X = new_x;
        player->Y = new_y;
        player->increment_move_no();

        // Push the new move onto the stack
        stack->Push('P', player->X, player->Y);

        // Update the last move to the current direction
        last_move = input;
    }

    // overwrites the grid cells to show updated position
    void adjustingPlayer_onGrid()
    {
        // previous osition is filled with .
        player_prevPos->data = '.';

        // if player moved up
        if (player->X == player_prevPos->row - 1)
        {
            player_prevPos->up->data = 'P';
            // update the players new posiion as prev
            player_prevPos = player_prevPos->up;
        }

        // if player moved down
        else if (player->X == player_prevPos->row + 1)
        {
            player_prevPos->down->data = 'P';
            // update the players new posiion as prev
            player_prevPos = player_prevPos->down;
        }

        // if player moved left
        else if (player->Y == player_prevPos->col - 1)
        {
            player_prevPos->left->data = 'P';
            // update the players new posiion as prev
            player_prevPos = player_prevPos->left;
        }

        // if player moved right
        else if (player->Y == player_prevPos->col + 1)
        {
            player_prevPos->right->data = 'P';
            // update the players new posiion as prev
            player_prevPos = player_prevPos->right;
        }
        // no change in position
        else
            player_prevPos->data = 'P';
    }

    void displayGrid()
    {
        // clear();
        mvprintw(1, 50, "LEVEL: %d", level);
        remaining_moves = player->move_no < moves ? moves - player->move_no : 0;
        mvprintw(2, 20, "Remaining Moves: %d", remaining_moves);
        mvprintw(2, 70, "Remaining Undo Moves: %d",
                 undoMoves - stack->undoMovesLeft());
        mvprintw(3, 20, "Score: %d", score);
        mvprintw(3, 70, "key status: %s", (key.status == 1 ? "True" : "False"));

        hintSystem();

        // player's new position updation
        adjustingPlayer_onGrid();

        GridCell *row = head;
        grid_Xcor = 7;
        while (row != nullptr)
        {
            grid_Ycor = 35;
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
        mvprintw(grid_Xcor + 2, 10,
                 "using up, down, left, right arrowkeys  or "
                 " press u for undo or "
                 " press esc to quit): ");
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
        mvprintw(5, 20, "HINT: ");
        if (player_key_dist <= 3)
            mvprintw(5, 40, "Getting Closer");
        else
            mvprintw(5, 40, "Further Away");
    }

    int random()
    {
        int a = 1122;
        int c = 12345;
        // using LCG formula
        seed = ((a * seed + c) % dimension) + 1;
        return seed;
    }
};

int main()
{
    initscr(); // Initialize the screen

    // level adjustment menu
    int level;
    mvprintw(5, 50, "select level: ");
    mvprintw(6, 50, "1. Easy ");
    mvprintw(7, 50, "2. Medium ");
    mvprintw(8, 50, "3. Hard ");
    mvprintw(9, 50, "Enter 1,2,3: ");
    level = getch() - '0'; // getch gets the assci code of the key pressed

    // checking if valid input
    if (level < 1 || level > 3)
    {
        mvprintw(10, 50, "Invalid input! Please enter 1, 2, or 3.");
        refresh();
        getch();
        endwin();
        return 0; // exits the game
    }

    Grid G(level);
    clear(); // clear the screen before making the grid
    G.makGrid();
    G.stack->display();

    int input;
    while (true)
    {
        refresh(); // Refreshes the screen to see the updates

        // displaying grid
        G.displayGrid();

        // asking for user input
        mvprintw(G.grid_Xcor + 4, 10, "enter movement : ");
        input = getch();

        // player movement
        G.player_movement(input);

        // i.e esc key or moves are completed
        if (input == 27 || G.remaining_moves == 0)
        {
            clear();
            mvprintw(7, 50, "GAME OVER!");
            getch();
            break;
        }

        // display move history
        G.stack->display();
    }

    endwin();
    // endwin() ends the "curses mode" and brings the terminal back to normal
    return 0;
}