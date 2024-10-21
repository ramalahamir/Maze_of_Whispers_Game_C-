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

    char symbol; // for invisible items (bomb, key, door)

    GridCell(int r, int c, char val)
    {
        data = val;
        row = r;
        col = c;
        symbol = '.'; // by default
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

// making Node and list library for undoStack
struct Node
{
    coordinates cor;
    Node *next;

    Node(int r, int c)
    {
        cor.row = r;
        cor.col = c;
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

class Queue
{
  private:
    Node *front, *rear;

  public:
    Queue() { front = rear = nullptr; }

    bool isEmpty() { return front == nullptr; }

    void enqueue(int row, int col)
    {
        Node *newNode = new Node(row, col);
        if (isEmpty())
            front = rear = newNode;
        else
        {
            rear->next = newNode;
            rear = newNode;
        }
    }

    coordinates dequeue()
    {
        if (isEmpty())
            return coordinates(-1, -1);

        coordinates cor = front->cor;

        Node *temp = front;
        front = front->next;
        delete temp;

        return cor;
    }

    void displayQueue()
    {
        if (isEmpty())
        {
            mvprintw(11, 20, "Queue is empty");
            return;
        }

        Node *temp = front;
        int line = 11;
        while (temp != nullptr)
        {
            mvprintw(line, 20, "(C, %d, %d)", temp->cor.row, temp->cor.col);
            temp = temp->next;
            line++;
        }
    }

    Node *peek() { return front; }

    ~Queue()
    {
        while (!isEmpty())
        {
            dequeue();
        }
    }
};

class Stack
{
    int size;
    int capacity;

  public:
    Node *top;

    Stack() {}
    Stack(int cap)
    {
        top = nullptr;
        size = 0;       // counter
        capacity = cap; // fixed size linked list
    }

    ~Stack()
    {
        Node *temp = top;
        while (temp != nullptr)
        {
            Node *del = temp;
            temp = temp->next;
            delete (del);
        }
    }

    void Push(int row, int col)
    {
        Node *temp = new Node(row, col);

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
        if (isEmpty() || size == capacity)
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
    int Size() { return size; }
    void updateCapacity(int n) { capacity += n; }

    // displaying Stack
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
            // Print the row and col of each node in the Stack
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
    bool invalid_move;
    bool gameover; // for bombs
    bool gamewin;

    int initial_CBD_key;  // initial distance between key and player
    int initial_CBD_door; // initial distance between door and player

    int player_key_dist;
    int player_door_dist;
    int key_door_dist;

    Key key;
    Player *player;
    Door door;

    // keeps track of the players old position
    GridCell *player_prevPos;
    Stack *undoStack;
    Stack *coinsStack;
    Stack *bombStack;
    Queue *coinCollection;

    // stack sizes
    int coinSize;
    int bombSize;

    unsigned long long int seed; // for the random function
    int coinChangeCounter;

    // starting grid X cor and Ycor on screen
    // this will be stored for display setting purposes
    int grid_Xcor, grid_Ycor;

    Grid(int lvl, int random_assci)
    {
        level = lvl;
        coinChangeCounter = score = 0;
        player = new Player();
        player_prevPos = nullptr;
        invalid_move = gameover = gamewin = false;
        coinCollection = new Queue();

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
        seed = random_assci;

        // setting the random positions for player, key and door
        key.key_x = random();
        key.key_y = random();
        player->X = random();
        player->Y = random();
        door.door_x = random();
        door.door_y = random();

        // setting the coin and bomb population number
        coinSize = dimension;
        bombSize = dimension / 2;
        bombStack = coinsStack = nullptr;

        // setting the bombs
        settingBombsPosition();

        // coin placements
        settingCoinsPosition();

        // calculating distance differences for player, key and door
        setting_distance_differences();

        // setting the initial distances
        initial_CBD_key = player_key_dist;
        initial_CBD_door = player_door_dist;

        // for moves
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

        // intitalizing undoStack size
        undoStack = new Stack(undoMoves);

        head = nullptr;
        tail = nullptr;
    }

    void settingCoinsPosition()
    {
        // delete the previous stack
        if (coinsStack != nullptr)
            delete coinsStack;
        coinsStack = new Stack(coinSize);

        // setting the new coins
        for (int i = 0; i < coinSize; i++)
        {
            coordinates coin_cor;
            // coins should not overlap the bombs
            do
            {
                coin_cor = setCoordinates(coinsStack);
            } while (
                overlappingThemselves(coin_cor.row, coin_cor.col, bombStack));

            // push the unique coordinate
            coinsStack->Push(coin_cor.row, coin_cor.col);
        }
    }

    void settingBombsPosition()
    {
        // delete the previous stack
        if (bombStack != nullptr)
            delete bombStack;
        bombStack = new Stack(bombSize);

        // setting the new bombs
        for (int i = 0; i < bombSize; i++)
        {
            coordinates bomb_cor = setCoordinates(bombStack);
            bombStack->Push(bomb_cor.row, bomb_cor.col);
        }
    }

    bool overlappingThemselves(int row, int col, Stack *&stack)
    {
        // compare the already present items coordinates
        if (stack->top == nullptr)
            return false;

        Node *temp = stack->top;
        while (temp != nullptr)
        {
            // overlaps
            if (temp->cor.row == row && temp->cor.col == col)
                return true;
            temp = temp->next;
        }
        return false; // if its unique
    }

    // coordinates that don't overwrite the player, key
    // or door
    coordinates setCoordinates(Stack *&stack)
    {
        coordinates cor;
        do
        {
            cor.row = random();
            cor.col = random();

        } while ((cor.row == player->X && cor.col == player->Y) ||
                 (cor.row == key.key_x && cor.col == key.key_y) ||
                 (cor.row == door.door_x && cor.col == door.door_y) ||
                 (overlappingThemselves(cor.row, cor.col, stack)));

        return cor;
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
        bool cell_filled;

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                cell_filled = false;

                // setting the up and down boundaries
                if (i == 0 || j == 0 || i == rows - 1 || j == cols - 1)
                {
                    cell = new GridCell(i, j, '#');
                    cell_filled = true;
                }

                // setting the player on the grid
                else if (i == player->X && j == player->Y)
                {
                    cell = new GridCell(i, j, 'P');
                    player_prevPos = cell;
                    undoStack->Push(player_prevPos->row, player_prevPos->col);
                    cell_filled = true;
                }

                // setting the key on the grid
                else if (i == key.key_x && j == key.key_y)
                {
                    // key is invisible
                    cell = new GridCell(i, j, '.');
                    cell->symbol = key.symb;
                    cell_filled = true;
                }

                // setting the door on the grid
                else if (i == door.door_x && j == door.door_y)
                {
                    // key is invisible
                    cell = new GridCell(i, j, '.');
                    cell->symbol = door.symb;
                    cell_filled = true;
                }

                // if not filled yet
                else if (cell_filled == false)
                {
                    // place the coins
                    Node *temp = coinsStack->top;
                    while (temp != nullptr)
                    {
                        if (i == temp->cor.row && j == temp->cor.col)
                        {
                            cell = new GridCell(i, j, 'C');
                            cell_filled = true;
                            break;
                        }
                        temp = temp->next;
                    }

                    // if no coin fills check for bomb placements
                    if (cell_filled == false)
                    {
                        Node *temp = bombStack->top;
                        while (temp != nullptr)
                        {
                            if (i == temp->cor.row && j == temp->cor.col)
                            {
                                // bombs are invisible
                                cell = new GridCell(i, j, '.');
                                // set the bomb state of the cell true
                                cell->symbol = 'B';
                                cell_filled = true;
                                break;
                            }
                            temp = temp->next;
                        }
                    }
                }

                // regular cell
                if (cell_filled == false)
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
        invalid_move = false;

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
                if (player->X > 0)
                    new_x = player->X - 1;
                break;

            // DOWN
            case 's':
            case 'S':
                if (last_move == 'w') // if the last move was up
                    polarOpposite = true;
                if (player->X <= dimension)
                    new_x = player->X + 1;
                break;

            // LEFT
            case 'a':
            case 'A':
                if (last_move == 'd') // if the last move was right
                    polarOpposite = true;
                if (player->Y > 0)
                    new_y = player->Y - 1;
                break;

            // RIGHT
            case 'd':
            case 'D':
                if (last_move == 'a') // if the last move was left
                    polarOpposite = true;
                if (player->X <= dimension)
                    new_y = player->Y + 1;
                break;

            // Undo movement
            case 'u':
            case 'U':
            {
                if (!undoStack->isEmpty())
                {
                    // pop the current move since forward history doesn't matter
                    undoStack->Pop();

                    // after popping the top most element, below are the new
                    // coordinates
                    coordinates prev_move = undoStack->Pop();

                    // if move exists only then undo
                    if (prev_move.row != -1 && prev_move.row != -1)
                    {
                        player->X = prev_move.row;
                        player->Y = prev_move.col;

                        // Push the new move onto the undoStack
                        undoStack->Push(player->X, player->Y);
                    }
                }
                return;
            }

            default:
                invalid_move = true;
                break;
        }

        // Check if move is within grid boundaries
        if (new_x <= 0 || new_x >= dimension + 1 || new_y <= 0 ||
            new_y >= dimension + 1)
            invalid_move = true;

        // If the move is polar opposite, don't allow it without undo
        if (polarOpposite)
            invalid_move = true;

        if (invalid_move)
            return;

        // Update player position if valid move
        player->X = new_x;
        player->Y = new_y;
        player->increment_move_no();

        // Push the new move onto the undoStack
        undoStack->Push(player->X, player->Y);

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
            // check for interaction after each movement
            player_item_interaction(player_prevPos->up);

            player_prevPos->up->data = 'P';
            // update the players new posiion as prev
            player_prevPos = player_prevPos->up;
        }

        // if player moved down
        else if (player->X == player_prevPos->row + 1)
        {
            // check for interaction after each movement
            player_item_interaction(player_prevPos->down);

            player_prevPos->down->data = 'P';
            // update the players new posiion as prev
            player_prevPos = player_prevPos->down;
        }

        // if player moved left
        else if (player->Y == player_prevPos->col - 1)
        {
            // check for interaction after each movement
            player_item_interaction(player_prevPos->left);

            player_prevPos->left->data = 'P';
            // update the players new posiion as prev
            player_prevPos = player_prevPos->left;
        }

        // if player moved right
        else if (player->Y == player_prevPos->col + 1)
        {
            // check for interaction after each movement
            player_item_interaction(player_prevPos->right);

            player_prevPos->right->data = 'P';
            // update the players new posiion as prev
            player_prevPos = player_prevPos->right;
        }
        // no change in position
        else
            player_prevPos->data = 'P';
    }

    void adjust_coins_position(bool remove = false)
    {
        // iterating the linked grid
        GridCell *temp = head;

        // iterates the rows
        while (temp->down != nullptr)
        {
            // iterates the columns
            GridCell *cell = temp;
            while (cell != nullptr)
            {

                // checking for coins
                // iterating the coin stack
                Node *coin = coinsStack->top;
                while (coin != nullptr)
                {
                    // displaying C at the new coordinates
                    if (cell->row == coin->cor.row &&
                        cell->col == coin->cor.col)
                    {
                        // remove is the state that is deciding whether the
                        // function is removing the old coins or adding the new
                        // coins
                        if (remove)
                            cell->data = '.';
                        else
                            cell->data = 'C';
                        break;
                    }
                    coin = coin->next;
                }

                cell = cell->right;
            }
            temp = temp->down;
        }
    }

    void player_item_interaction(GridCell *&player_pos_cell)
    {
        // if valid move check for coin, key, door, bomb interaction
        if (!invalid_move)
        {
            // if the new player position had the coin
            // add the coordinates to the queue
            if (player_pos_cell->data == 'C')
            {
                coinCollection->enqueue(player->X, player->Y);

                // player gets 2 points and an undo move
                score += 2;
                undoMoves++;
                undoStack->updateCapacity(1);
                return;
            }

            // checking if the symbol was a bomb because bombs are invisible
            if (player_pos_cell->symbol == 'B')
            {
                // game ends
                gameover = true;
                return;
            }

            // check for key interaction
            if (player->X == key.key_x && player->Y == key.key_y)
            {
                // key obtained indicator
                key.status = true;
                return;
            }

            // check for door interaction
            if (player->X == door.door_x && player->Y == door.door_y &&
                key.status == true)
            {
                gamewin = true;
                return;
            }
        }
    }

    void displayGrid()
    {
        clear();

        // player's new position updation
        adjustingPlayer_onGrid();

        mvprintw(1, 50, "LEVEL: %d", level);
        remaining_moves = player->move_no < moves ? moves - player->move_no : 0;
        mvprintw(2, 20, "Remaining Moves: %d", remaining_moves);
        mvprintw(2, 70, "Remaining Undo Moves: %d",
                 undoMoves - undoStack->Size());
        mvprintw(3, 20, "Score: %d", score);
        mvprintw(3, 70, "key status: %s", (key.status == 1 ? "True" : "False"));

        // hint system for key and door
        if (key.status == false)
            hintSystem(player_key_dist, initial_CBD_key);
        else
            hintSystem(player_door_dist, initial_CBD_door);

        // change the coins position based on the counter
        // i.e change after every 5 moves
        coinChangeCounter++;
        if (coinChangeCounter == 5)
        {
            // before resetting the new coins
            // remove the previous coins in the grid
            adjust_coins_position(true);
            // reset the coins
            settingCoinsPosition();
            // add the new coins to the grid
            adjust_coins_position();

            coinChangeCounter = 0;
        }

        // calculate CBD's after each movement!
        setting_distance_differences();

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

        // print to notify the player entered wrong move
        if (invalid_move)
        {
            mvprintw(grid_Xcor + 5, 10, "Invalid move!");
            refresh();
            getch();
            noecho();
        }

        // display move history
        undoStack->display();
        refresh();
    }

    int cityBlockDistance(int x1, int y1, int x2, int y2)
    {
        int mod_x = (x2 - x1 < 0) ? x1 - x2 : x2 - x1;
        int mod_y = (y2 - y1 < 0) ? y1 - y2 : y2 - y1;

        int dist = mod_x + mod_y;
        return dist;
    }

    void hintSystem(int curr_dist, int init_dist)
    {
        mvprintw(5, 20, "HINT: ");
        // comparing the current and the initial distance
        if (curr_dist <= init_dist)
            mvprintw(5, 40, "Getting Closer");
        else
            mvprintw(5, 40, "Further Away");
    }

    int random()
    {
        int a = 1664525;
        unsigned long long int c = 1013904223;
        unsigned long long int m = 4294967296;

        // Update the seed
        seed = (a * seed + c) % m;
        return seed % dimension + 1;
    }

    void display_score()
    {
        // add the remaining moves into the score
        score += remaining_moves;

        mvprintw(9, 40, "your score: %d", score);
        mvprintw(10, 40, "coins collected: ");
        coinCollection->displayQueue();
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
    // user has to press a button again to enter the game and we'll use that as
    // the seed
    int random_ascii = getch();

    // checking if valid input
    if (level < 1 || level > 3)
    {
        mvprintw(10, 50, "Invalid input! Please enter 1, 2, or 3.");
        refresh();
        getch();
        endwin();
        return 0; // exits the game
    }

    Grid G(level, random_ascii);
    clear(); // clear the screen before making the grid
    G.makGrid();

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
        if (input == 27 || G.remaining_moves == 0 || G.gameover)
        {
            clear();
            mvprintw(7, 50, "GAME OVER!");
            if (G.remaining_moves == 0)
                mvprintw(8, 40, "you ran out of moves");
            if (G.gameover)
                mvprintw(8, 40, "you stepped on a bomb");
            G.display_score();
            getch();
            break;
        }

        if (G.gamewin)
        {
            clear();
            mvprintw(7, 50, "YOU WON!");
            G.display_score();
            getch();
            break;
        }
    }
    endwin();
    // endwin() ends the "curses mode" and brings the terminal back to normal
    return 0;
}