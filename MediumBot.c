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

typedef struct
{

    EasyBot base;

    int ConsecutiveMisses; /*tracks consecutive misses to see if the bot is struggling
                             should be 0-ed in the main if the bot hits */
    int radaredCoord[5][2];

} MediumBot;

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
    // if last move was radar we check the 2X2 grid of which hit_coord is the top left corner

    else if (easyBot->last_move == 2)
    {

        int possibilities[10][2] = {{row, col}, {row + 1, col}, {row, col + 1}, {row + 1, col + 1}};

        for (int i = 0; i < 4; i++)
        {
            int r = possibilities[i][0];
            int c = possibilities[i][1];
            addto_hit_stack(easyBot, r, c, PlayerGrid);
        }
    }

    // if last move was artilery we check eaxh cell in the 2X2 grid of which hit_coord is the top left corner
    // if each cell was a hit or not

    else if (easyBot->last_move == 3)
    {

        int possibilities[10][2] = {{row, col}, {row + 1, col}, {row, col + 1}, {row + 1, col + 1}};

        for (int i = 0; i < 4; i++)
        {

            int r = possibilities[i][0];
            int c = possibilities[i][1];

            if (PlayerGrid[r][c] == '*')
            {
                check_around_hit(easyBot, PlayerGrid, r, c);
            }
        }
    }

}


bool should_use_radar(MediumBot *mediumBot, char PlayerGrid[10][10])
{

    if (mediumBot->base.base.radarUses > 0 && mediumBot->ConsecutiveMisses >= 3)
    {
        return 1; // struggling to find targets -> use radar
    }

    return 0;
}
bool isCoordFoundInArray(int Coord[5][2], int row, int col)
{
    bool isFound = false;
    for (int i = 0; i < 5; i++)
    {
        if (Coord[i][0] == row && Coord[i][1] == col)
            isFound = true;
    }
    return isFound;
}

// used for radar and artillery
void find_best_2by2_area(int radaredCoord[5][2], char PlayerGrid[10][10], int *row, int *col)
{
    int max_water = -1;

    // the following array can fit 25 grids stored with their top_left coordinate
    int potential_areas[25][2];
    int potential_count = 0;

    for (int r = 0; r < 9; r++)
    {
        for (int c = 0; c < 9; c++)
        {

            if (!isCoordFoundInArray(radaredCoord, r, c))
            {

                int water = 0;

                for (int r1 = r; r1 < r + 2; r1++)
                {
                    for (int c1 = c; c1 < c + 2; c1++)
                    {
                        if (PlayerGrid[r1][c1] == '~')
                        {
                            water += 1;
                        }
                    }
                }

                if (water > max_water)
                {
                    max_water = water;
                    potential_count = 0;
                    potential_areas[potential_count][0] = r;
                    potential_areas[potential_count][1] = c;
                    potential_count++;
                }
                else if (water == max_water)
                {
                    potential_areas[potential_count][0] = r;
                    potential_areas[potential_count][1] = c;
                    potential_count++;
                }
            }
        }
    }

    srand(time(NULL));
    int random_index = rand() % potential_count;
    *row = potential_areas[random_index][0];
    *col = potential_areas[random_index][1];
}

void generate_medium_bot_move(MediumBot *mediumBot, char PlayerGrid[10][10], char move[9], int *row, int *col)
{

    if (mediumBot->base.hit_stack_size == 0)
        mediumBot->base.haunting = true;
    // if the medium bot is not haunting

    if (mediumBot->base.base.Artillery)

    {
        find_best_2by2_area(mediumBot->radaredCoord, PlayerGrid, row, col);
        strcpy(move, "Artillery");
        mediumBot->base.last_move = 3;
    }

    else if (!mediumBot->base.haunting)
    {
        // we set the move to be fire
        strcpy(move, "Fire");
        // we concatentate to the move the coordinates found last in the hit stack
        // we decrement the hit stack size
        --mediumBot->base.hit_stack_size;
        *row = mediumBot->base.hit_stack[mediumBot->base.hit_stack_size][0];
        *col = mediumBot->base.hit_stack[mediumBot->base.hit_stack_size][1];
    }

    // else if we are haunting for places to target
    // and we can still use radar and it is a good choice to make use of it in this turn
    else if (should_use_radar(mediumBot, PlayerGrid))
    {

        // finds the best coordiates to tagret by radar and stores it in radar target
        find_best_2by2_area(mediumBot->radaredCoord, PlayerGrid, row, col);
        // we decrement the number of radar sweeps we can still use
        //  move  = Radar + radar target
        strcpy(move, "Radar");
        mediumBot->base.last_move = 2;
        mediumBot->ConsecutiveMisses = 0;
    }
    // if we're haunting but it's not a good choice to use radar
    else
    {
        // we'll have to make a guess but not a completely random one
        do
        {
            *col = rand() % GRID_SIZE;
            *row = rand() % GRID_SIZE;
        } while (PlayerGrid[*row][*col] != '~');
        strcpy(move, "Fire");
        mediumBot->base.last_move = 1;
    }
}