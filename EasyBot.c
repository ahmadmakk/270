#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define GRID_SIZE 10
#include <stdbool.h>
#include <string.h>
#include <unistd.h> // for the sleep func
#include <windows.h>
/* display: to be attacked by the oppoenet player (~ or * or 0)
hide: to keep record of where player's ships are (letter of the ship or ~)*/
int difficulty;

typedef struct grid
{
    char display[10][10];
    char hide[10][10];
} Grid;

typedef struct ship
{
    char *name;
    int cells;
    int hitCount; // we need it to check later if the ship has been sunk or no
} Ship;

typedef struct player
{
    bool smoked[GRID_SIZE][GRID_SIZE];
    char *name;
    int isTurn;
    int countSunk;
    int radarUses;
    int smokeUses;
    bool Artillery;
    bool Torpedo;
    Grid own;
    Ship ships[4];

} Player;

typedef struct
{

    Player base;

    bool haunting;
    int hit_stack[100][2];
    int hit_stack_size;
    int sunk;
    int orientation;
    int last_move;
    int troubleCells[2][2];

} EasyBot;




int validateCoordinates(int row, int column)
{
    return (row >= 0 && row < 10 && column >= 0 && column < 10);
}
int letterToNumber(char letter)
{
    return letter - 'A';
}

int isShipSunk(Ship ship)
{
    return ship.hitCount == ship.cells;
}



void addto_hit_stack(EasyBot *easyBot, int row, int col, char PlayerGrid[10][10])
{

    if (validateCoordinates(row, col) && PlayerGrid[row][col] == '~')
    {
        easyBot->hit_stack[easyBot->hit_stack_size][0] = row;
        easyBot->hit_stack[easyBot->hit_stack_size][1] = col;
        easyBot->hit_stack_size++;
    }
}

void check_around_hit(EasyBot *easyBot, char PlayerGrid[10][10], int row, int col)
{

    int down_Up_left_right[4][2] = {{row - 1, col}, {row + 1, col}, {row, col - 1}, {row, col + 1}};

    if (easyBot->orientation == 0)
    {
        easyBot->troubleCells[0][0] = row;
        easyBot->troubleCells[0][1] = col;

        if (validateCoordinates(row + 1, col) && PlayerGrid[row + 1][col] == '*')
        {
            easyBot->orientation = 1;
            easyBot->troubleCells[0][0] = row + 1;
            easyBot->troubleCells[0][1] = col;
        }
        else if (validateCoordinates(row - 1, col) && PlayerGrid[row - 1][col] == '*')
        {
            easyBot->orientation = 1;
            easyBot->troubleCells[0][0] = row - 1;
            easyBot->troubleCells[0][1] = col;
        }

        else if (validateCoordinates(row, col + 1) && PlayerGrid[row][col + 1] == '*')
        {
            easyBot->orientation = 2;
            easyBot->troubleCells[0][0] = row;
            easyBot->troubleCells[0][1] = col + 1;
        }
        else if (validateCoordinates(row, col - 1) && PlayerGrid[row][col - 1] == '*')
        {
            easyBot->orientation = 2;
            easyBot->troubleCells[0][0] = row;
            easyBot->troubleCells[0][1] = col - 1;
        }
    }

    // no orientation
    if (easyBot->orientation == 0)
    {
        addto_hit_stack(easyBot, row + 1, col, PlayerGrid);
        addto_hit_stack(easyBot, row, col + 1, PlayerGrid);
        addto_hit_stack(easyBot, row - 1, col, PlayerGrid);
        addto_hit_stack(easyBot, row, col - 1, PlayerGrid);
    }
    // vertical orientation
    if (easyBot->orientation == 1)
    {

        addto_hit_stack(easyBot, row - 1, col, PlayerGrid);
        addto_hit_stack(easyBot, row + 1, col, PlayerGrid);
    }
    // horizontal orienatation
    if (easyBot->orientation == 2)
    {
        addto_hit_stack(easyBot, row, col - 1, PlayerGrid);
        addto_hit_stack(easyBot, row, col + 1, PlayerGrid);
    }
}


void update_bot_state_after_hit(EasyBot *easyBot, int row, int col, char PlayerGrid[10][10])
{

    easyBot->haunting = false;

    // if last move was fire we check adjacent cells depending on orientation
    if (easyBot->last_move == 1)
    {
        check_around_hit(easyBot, PlayerGrid, row, col);
    }
    
}

void generate_easy_bot_move(EasyBot *easyBot, char PlayerGrid[10][10], int *row, int *col)
{
    if (easyBot->hit_stack_size == 0)
        easyBot->haunting = true;

    // Update the bot's sunk count from the player
    easyBot->sunk = easyBot->base.countSunk;

    if (easyBot->haunting)
    {
        // Randomly generate coordinates while searching
        do
        {
            *col = rand() % GRID_SIZE;
            *row = rand() % GRID_SIZE;
        } while (PlayerGrid[*row][*col] != '~'); // Only target unexplored positions
    }
    else
    {
        // Target stored coordinates from the hit stack
        if (easyBot->hit_stack_size > 0)
        {
            *row = easyBot->hit_stack[easyBot->hit_stack_size - 1][0];
            *col = easyBot->hit_stack[easyBot->hit_stack_size - 1][1];

            // Remove the coordinate from the stack
            easyBot->hit_stack_size--;
        }
    }
}
