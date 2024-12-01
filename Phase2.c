// This project was done by: Nermine Chhayta, Nour Fneish, Ahmad Makkouk and Hayat Bounite
// Team: World war III
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> // for the sleep func
#include <windows.h>
#define GRID_SIZE 10

int difficulty; // 0 for easy, 1 for medium and 2 for hard

typedef struct grid
{
    char display[10][10]; // to be attacked by the opponent player (~ or * or 0 or the corresponding letter of a sunken ship)
    char hide[10][10];    // to keep record of where player's ships are (letter of the ship or ~)
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

    int ConsecutiveMisses; // tracks consecutive misses to see if the bot is struggling (to use Radar)
    int radaredCoord[5][2];

} MediumBot;

typedef struct
{
    MediumBot base;

    int weightGrid[10][10];
    bool smokedShips[5];
    int smokeUsed;

} HardBot;

// for the user to fell the movements of the bots realistic in time and to be able to track the changes
void sleepp(int sec)
{
    time_t start_time = time(NULL);
    while (time(NULL) - start_time < sec)
    {
        // Busy wait
    }
}

// Check if the row and column are valid
int validateCoordinates(int row, int column)
{
    return (row >= 0 && row < 10 && column >= 0 && column < 10);
}

void printGrid(char arr[10][10])
{
    for (int i = -1; i < 10; i++)
    {
        for (int j = -1; j < 10; j++)
        {
            if (i == -1 && j >= 0)
            {
                printf("%c ", 'A' + j);
            }
            else if (j == -1 && i >= 0)
            {
                printf("%2d ", i + 1);
            }
            else if (i >= 0 && j >= 0)
            {
                printf("%c ", arr[i][j]);
            }
            else if (i == -1 && j == -1)
            {
                printf("   ");
            }
        }
        printf("\n");
    }
}

// transforms the given letter to a 0-based indexing number
// this function will later help us in manipulating the columns as they are represented by letters in our grids
int letterToNumber(char letter)
{
    return letter - 'A';
}

// check whether a ship can be deployed by the player in the position specified
// returns 0 if the position if the placement can't occur
int check(char position[7], Player *player, int cells, char ship)
{

    //  Convert letter of colomn to index
    int column = letterToNumber(position[0]);
    char direction;
    // here we just decremt 1 from the row to make it aligned with 0 indexing
    int row;
    if (position[1] == '1' && position[2] == '0')
    { // Specifically handle "10"
        row = 9;
        direction = position[4];
    }
    else if (position[1] >= '1' && position[1] <= '9' && (position[2] == ',' || position[2] == 0))
    {
        row = position[1] - '1';
        direction = position[3];
    }
    else
    {
        printf("Invalid row number!\n");
        return 0;
    }

    // Check if row and column are within bounds so between 0 and 10
    if (!validateCoordinates(row, column))
    {
        printf("Position provided out of the grid!\n");
        return 0; // unvalid position
    }

    // Check for placement based on direction

    if (direction == 'h')
    {

        // Check if placing horizontally would go out of bounds
        if (column + cells > 10)
        {
            printf("Position provided out of the grid! \n");
            return 0; // unvalid position
        }

        // Check if the cells are available
        for (int i = 0; i < cells; i++)
        {
            if (player->own.hide[row][column + i] != '~') // since '~' means available
            {
                printf("Cell is occupied!\n");
                return 0; // Cell is occupied so unvalid position
            }
        }

        // position is valid so we place it
        for (int i = 0; i < cells; i++)
        {
            player->own.hide[row][column + i] = ship; // Place the ship by adding its initial
        }
    }

    else if (direction == 'v')
    {
        if (row + cells > 10)
        {
            printf("Position provided out of the grid! \n");
            return 0;
        }
        for (int i = 0; i < cells; i++)
        {
            if (player->own.hide[row + i][column] != '~')
            {
                printf("Cell is occupied!\n");
                return 0;
            }
        }
        for (int i = 0; i < cells; i++)
        {
            player->own.hide[row + i][column] = ship;
        }
    }
    // direction is not vertical or horizontal!unvalid
    else
    {
        printf("Invalid direction!\n");
        return 0;
    }

    printf("Placement successful.\n");
    return 1; // Success
}

// Function to switch the turn between the player and the bot
void switchTurns(Player *p1, Player *p2)
{
    if (p1->isTurn) // p1 turn is 1
    {
        p1->isTurn = 0;
        p2->isTurn = 1;
    }
    else
    {
        p1->isTurn = 1;
        p2->isTurn = 0;
    }
}

int isShipSunk(Ship ship)
{
    return ship.hitCount == ship.cells;
}

bool Radar(Player *attacker, Player *opponent, int topLeftRow, int topLeftCol)
{

    // Disable Torpedo and Artillery abilities
    attacker->Torpedo = false;
    attacker->Artillery = false;

    // Check if there are any radar scans left
    if (attacker->radarUses <= 0)
    {
        printf("No radar scans left! You lose your turn.\n");
        return false;
    }

    // Validate if the 2x2 area is within grid bounds
    if (!validateCoordinates(topLeftRow + 1, topLeftCol + 1))
    {
        printf("Invalid radar scan area! You lose your turn.\n");
        return false;
    }

    // Check the 2x2 area for any ships
    bool shipsFound = false;

    for (int i = topLeftRow; i < 2 + topLeftRow; i++)
    {
        for (int j = topLeftCol; j < 2 + topLeftCol; j++)
        {
            // Skip cells covered by smoke screens
            if (opponent->smoked[i][j])
            {
                continue;
            }

            char currentCell = opponent->own.hide[i][j];
            if (currentCell == 'c' || currentCell == 'b' || currentCell == 'd' || currentCell == 's')
            {
                shipsFound = true;
                break; // Stop checking once a ship is found
            }
        }

        if (shipsFound)
            break;
    }

    // Display result
    if (shipsFound)
    {
        printf("Enemy ships found in the area!\n");
    }
    else
    {
        printf("No enemy ships found in the area.\n");
    }

    // Decrement radar use count
    attacker->radarUses--;

    return shipsFound;
}

void Smoke(Player *attacker, int topLeftRow, int topLeftCol)
{

    // Disable artillery when using a smoke screen
    attacker->Artillery = false;
    attacker->Torpedo = false;

    // Validate if the 2x2 area is within grid bounds
    if (!validateCoordinates(topLeftRow + 1, topLeftCol + 1))
    {
        printf("Invalid smoke screen area! You lose your turn and 1 smoke use.\n");
        return;
    }

    // Check if smoke screens are available
    if (attacker->smokeUses <= 0)
    {
        printf("No smoke screens left. Turn skipped.\n");
        return;
    }

    // Place smoke screen in the 2x2 area
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            attacker->smoked[topLeftRow + i][topLeftCol + j] = true;
        }
    }

    // Clear screen for next player's turn (optional, mainly for console-based games)
    // system("cls");

    printf("Smoke screen placed successfully!\n");

    // Decrement smoke screen count after successful placement
    attacker->smokeUses--;
}

bool artilleryAttack(Player *attacker, Player *opponent, int topLeftRow, int topLeftCol)
{
    if (!attacker->Artillery)
    {
        printf("Artillery move not unlocked!\n");
        return false;
    }
    int sunkshipprev = attacker->countSunk;

    // Validate if the 2x2 area is within grid bounds
    if (!validateCoordinates(topLeftRow + 1, topLeftCol + 1))
    {
        printf("Invalid artillery target area! You lose your turn.\n");
        return false;
    }

    bool hit = false;

    // Check each cell in the 2x2 area
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            int row = topLeftRow + i;
            int col = topLeftCol + j;
            char cellInHide = opponent->own.hide[row][col];
            char cellInDisplay = opponent->own.display[row][col];

            if (cellInHide != '~')
            { // It's a ship

                if (opponent->own.display[row][col] == '~')
                {
                    opponent->own.display[row][col] = '*';
                }
                hit = true; // Mark that a hit occurred
                for (int i = 0; i < 4; i++)
                {
                    if (cellInHide == opponent->ships[i].name[0]) // Check if the hit matches a ship's initial
                    {
                        opponent->ships[i].hitCount++; // Increment hit count for the ship

                        if (isShipSunk(opponent->ships[i])) // Check if the ship is sunk
                        {
                            printf("%s is sunk!", opponent->ships[i].name); // Print which ship was sunk
                            attacker->countSunk++;
                            attacker->Artillery = true;
                            attacker->smokeUses++;
                        }

                        if (attacker->countSunk == 3)
                        {
                            attacker->Torpedo = true;
                        }
                        break; // Stop checking once we find the correct ship
                    }
                }
            }
            else
            {
                opponent->own.display[row][col] = 'O';
            }
        }
    }

    if (hit)
    {
        printf("Hit!\n");
    }
    else
    {
        printf("Miss!\n");
    }

    if (sunkshipprev == attacker->countSunk)
    {
        attacker->Artillery = false;
        attacker->Torpedo = false;
    }

    // Show the opponent's grid after the attack
    printf("%s's grid after the artillery strike:\n", (*attacker).name);
    return hit;
}

// choice: 1 for row and 2 for column
bool torpedoStrike(Player *attacker, Player *opponent, int choice, int line)
{
    if (!attacker->Torpedo)
    {
        printf("Torpedo move not unlocked!\n");
        return false;
    }

    bool hit = false;

    if (choice == 1)
    {
        // Apply torpedo strike in the specified direction
        for (int j = 0; j < 10; ++j)
        {
            char cell = opponent->own.hide[line][j];
            if (cell != '~')

            { // Ship detected and not hit before
                hit = true;

                if (opponent->own.display[line][j] == '~')
                {
                    opponent->own.display[line][j] = '*';
                }

                // Increment the hit count for the specific ship
                for (int i = 0; i < 4; i++)
                {
                    if (cell == opponent->ships[i].name[0])
                    { // Match ship by initial
                        opponent->ships[i].hitCount++;

                        // Check if the ship is sunk
                        if (isShipSunk(opponent->ships[i]))
                        {
                            printf("You sunk a %s!\n", opponent->ships[i].name);
                            attacker->countSunk++;

                            // Grant abilities based on ship sink count
                            attacker->Artillery = true;
                            attacker->smokeUses++;
                            if (attacker->countSunk == 3)
                            {
                                attacker->Torpedo = true;
                            }
                        }
                        break;
                    }
                }
            }
            else if (cell == '~')
            {
                opponent->own.display[line][j] = 'O'; // Mark a miss
            }
        }
    }

    else if (choice == 2)
    { // Vertical (column) attack
        for (int j = 0; j < 10; ++j)
        {
            char cell = opponent->own.hide[j][line];
            if (cell != '~')
            { // Ship detected and not hit before
                hit = true;

                if (opponent->own.display[j][line] == '~')
                {
                    opponent->own.display[j][line] = '*';
                }

                // Increment the hit count for the specific ship
                for (int i = 0; i < 4; i++)
                {
                    if (cell == opponent->ships[i].name[0])
                    { // Match ship by initial
                        opponent->ships[i].hitCount++;

                        // Check if the ship is sunk
                        if (isShipSunk(opponent->ships[i]))
                        {
                            printf("You sunk a %s!\n", opponent->ships[i].name);
                            attacker->countSunk++;

                            // Grant abilities based on ship sink count
                            attacker->Artillery = true;
                            attacker->smokeUses++;
                            if (attacker->countSunk == 3)
                            {
                                attacker->Torpedo = true;
                            }
                        }
                        break;
                    }
                }
            }
            else if (cell == '~')
            {
                opponent->own.display[j][line] = 'O'; // Mark a miss
            }
        }
    }

    // Print result based on whether there was a hit
    if (hit)
    {
        printf("Torpedo hit!\n");
    }
    else
    {
        attacker->Artillery = false;
        printf("Torpedo miss.\n");
    }

    attacker->Torpedo = false;
    return hit;
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

bool fire(Player *attacker, Player *defender, int row, int column)
{
    bool hit = false;

    int sunkshipprev = attacker->countSunk;

    char cellinHide = defender->own.hide[row][column];
    char cellinDisplay = defender->own.display[row][column];

    if (cellinDisplay != '~')
    {
        printf("The cell is already targeted! \n");
    }

    else if (cellinHide != '~')
    {
        printf("Hit!\n");
        hit = true;
        defender->own.display[row][column] = '*'; // mark hit on the display grid

        for (int i = 0; i < 4; i++)
        {
            if (cellinHide == defender->ships[i].name[0]) // Check if the hit matches a ship's initial
            {
                defender->ships[i].hitCount++; // Increment hit count for the ship

                if (isShipSunk(defender->ships[i])) // Check if the ship is sunk
                {
                    printf("%s was sunk!\n", defender->ships[i].name); // Print which ship was sunk
                    attacker->countSunk++;
                    attacker->Artillery = true;
                    attacker->smokeUses++;
                }

                if (attacker->countSunk == 3)
                {
                    attacker->Torpedo = true;
                }
                break; // Stop checking once we find the correct ship
            }
        }
    }
    // else the cell was water in the hide grid and was not a ship
    else
    {
        printf("Miss!\n");
        defender->own.display[row][column] = 'O'; // mark miss on the display grid only in easy mode
    }

    if (sunkshipprev == attacker->countSunk)
    {
        attacker->Artillery = false;
        attacker->Torpedo = false;
    }

    return hit;
}

void convertSunkenToLetter(Player *player)
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {

            if (player->own.display[i][j] == '*')
            {
                char shipInitial = player->own.hide[i][j];
                char direction = 'v';
                int shipNum;
                if (player->own.display[i][j + 1] == '*' && player->own.hide[i][j + 1] == shipInitial)
                    direction = 'h';

                if (shipInitial == 'c')
                    shipNum = 0;
                if (shipInitial == 'b')
                    shipNum = 1;
                if (shipInitial == 'd')
                    shipNum = 2;
                if (shipInitial == 's')
                    shipNum = 3;

                if (isShipSunk(player->ships[shipNum]))
                {
                    if (direction == 'h')
                    {
                        for (int k = j; k < player->ships[shipNum].cells + j; k++)
                        {

                            player->own.display[i][k] = shipInitial;
                        }
                    }

                    else
                    {
                        for (int k = i; k < player->ships[shipNum].cells + i; k++)
                        {

                            player->own.display[k][j] = shipInitial;
                        }
                    }
                }
            }
        }
    }
}

bool takeTurn(Player *attacker, Player *defender, char move[9], int row, int col)
{
    bool hit = false;

    int sinkshipprev = attacker->countSunk;

    // we examine all the movements possibilities
    if (!strcmp("Fire", move))
    {
        printf("Fire selected!\n");
        hit = fire(attacker, defender, row, col);
    }
    else if (!strcmp("Radar", move))
    {
        printf("Radar Sweep selected!\n");
        hit = Radar(attacker, defender, row, col);
    }
    else if (!strcmp("Smoke", move))
    {
        printf("Smoke Screen selected!\n");
        Smoke(attacker, row, col);
    }
    else if (!strcmp("Artillery", move))
    {
        printf("Artillery Attack selected!\n");
        hit = artilleryAttack(attacker, defender, row, col);
    }
    else if (!strcmp("Torpedo", move))
    {
        printf("Torpedo selected!\n");
        hit = torpedoStrike(attacker, defender, row, col);
    }
    else
    {
        printf("Invalid move!\n");
    }

    if (sinkshipprev < attacker->countSunk)
    {
        convertSunkenToLetter(defender);
    }

    // Display updated opponent(defender) grid after the turn
    printf("Updated grid for %s:\n", defender->name);
    printGrid(defender->own.display);

    sleepp(3);
    // system("cls");
    return hit;
}

void positionShips(Player *player, Ship *shipss)
{
    char position[7];

    // first we print their grid (which's initially all water)
    printGrid(player->own.display);

    int i = 0;
    // a for loop to pass over all the ships in the ships array
    for (i; i < 4; i++)
    {
        // asking the player for the position and storing it
        printf("Please enter the position of the %s of size %d\n", shipss[i].name, shipss[i].cells);
        scanf("%5s", position);

        // checking if the position is valid with our check function
        if (!check(position, player, shipss[i].cells, shipss[i].name[0]))
        {

            //  printf("Invalid coordinates\n"); // should be informed in the function check
            i--; // decremnting i so we pass over the ship again and the player get to deploy it correct
            continue;
        }

        // after each placement the player is shown their grid
        printGrid(player->own.hide);
    }
}

void placeBotShipsRandomly(Player *player, Ship ships[])
{
    srand(time(NULL));

    // Iterate over all ships to place them on the grid
    for (int i = 0; i < 4; i++)
    {
        Ship currentShip = ships[i];
        int placed = 0;

        while (!placed)
        {
            // Generate random starting position and direction
            int row = rand() % 10;                   // Random row (0 to 9)
            int col = rand() % 10;                   // Random column (0 to 9)
            char direction = rand() % 2 ? 'h' : 'v'; // Random direction ('h' or 'v')

            bool valid = true;

            if (direction == 'h')
            {
                // Check if ship would go out of bounds horizontally
                if (col + currentShip.cells > 10)
                {
                    valid = false;
                }
                else
                {
                    // Check if all cells are available
                    for (int j = 0; j < currentShip.cells; j++)
                    {
                        if (player->own.hide[row][col + j] != '~')
                        {
                            valid = false;
                            break;
                        }
                    }
                }
            }
            else if (direction == 'v')
            {
                // Check if ship would go out of bounds vertically
                if (row + currentShip.cells > 10)
                {
                    valid = false;
                }
                else
                {
                    // Check if all cells are available
                    for (int j = 0; j < currentShip.cells; j++)
                    {
                        if (player->own.hide[row + j][col] != '~')
                        {
                            valid = false;
                            break;
                        }
                    }
                }
            }

            // If placement is valid, place the ship
            if (valid)
            {
                if (direction == 'h')
                {
                    for (int j = 0; j < currentShip.cells; j++)
                    {
                        player->own.hide[row][col + j] = currentShip.name[0];
                    }
                }
                else if (direction == 'v')
                {
                    for (int j = 0; j < currentShip.cells; j++)
                    {
                        player->own.hide[row + j][col] = currentShip.name[0];
                    }
                }
                placed = 1; // Mark as placed
            }
        }
    }
}

void extract_row_col(char coord[3], int *row, int *col)
{

    *col = letterToNumber(coord[0]);

    if (coord[1] == '1' && coord[2] == '0')
    {             // Checking for "10"
        *row = 9; // Since the grid is zero-indexed, row 10 becomes 9
    }
    else if (coord[1] >= '1' && coord[1] <= '9' && coord[2] != 0)
        *row = 20;
    else
    {
        *row = coord[1] - '1'; // For single-digit rows
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

// the following function populates the hit stack with the possible adjacent targets of
// a hit once there is one using fire (up left, )
// radar returns "true" meaning Enemy ships found in the area so we store them in the stack
// currently radar returns void so we should make it return bool for that
// artilley hits
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

    // last move was torpedo
    else if (easyBot->last_move == 4)
    {
        int possibilities[20][2];
        for (int i = 0; i < 10; i++)
        {
            if (row = 1)
            {
                possibilities[i][0] = col;
                possibilities[i][1] = i;
            }
            else
            {
                possibilities[i][0] = i;
                possibilities[i][1] = col;
            }
        }

        for (int i = 0; i < 10; i++)
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

void calculateWeight(int weightGrid[GRID_SIZE][GRID_SIZE], Player human)
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {

            if (human.own.display[i][j] == '~')
            {

                for (int k = 0; k < 4; k++)
                {
                    Ship ship = human.ships[k];

                    if (!isShipSunk(ship))
                    {
                        // check horizontal placement
                        if (j + ship.cells <= GRID_SIZE)
                        {
                            bool isMiss = false;
                            for (int r = j; r < j + ship.cells; r++)
                                if (human.own.display[i][r] != '~')
                                    isMiss = true;
                            if (!isMiss)
                                weightGrid[i][j]++;
                        }
                        // vertical placement
                        if (i + ship.cells <= GRID_SIZE)
                        {
                            bool isMiss = false;
                            for (int r = i; r < i + ship.cells; r++)
                                if (human.own.display[r][j] != '~')
                                    isMiss = true;
                            if (!isMiss)
                                weightGrid[i][j]++;
                        }
                    }
                }
            }
        }
    }
}

void selectTarget(int *row, int *col, int weightGrid[GRID_SIZE][GRID_SIZE])
{
    int maxWeigth = -1;

    int potential_cells[30][2];
    int potential_count = 0;

    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (weightGrid[i][j] > maxWeigth)
            {
                maxWeigth = weightGrid[i][j];
                potential_count = 0;
                potential_cells[potential_count][0] = i;
                potential_cells[potential_count][1] = j;
                potential_count++;
            }
            else if (weightGrid[i][j] == maxWeigth)
            {
                potential_cells[potential_count][0] = i;
                potential_cells[potential_count][1] = j;
                potential_count++;
            }
        }
    }

    srand(time(NULL));
    int random_index = rand() % potential_count;
    *row = potential_cells[random_index][0];
    *col = potential_cells[random_index][1];
}

void selectTargetINbounds(int *row, int *col, int weightGrid[GRID_SIZE][GRID_SIZE])
{
    int maxWeigth = -1;

    int potential_cells[30][2];
    int potential_count = 0;

    for (int i = 1; i < GRID_SIZE - 1; i++)
    {
        for (int j = 1; j < GRID_SIZE - 1; j++)
        {
            if (weightGrid[i][j] > maxWeigth)
            {
                maxWeigth = weightGrid[i][j];
                potential_count = 0;
                potential_cells[potential_count][0] = i;
                potential_cells[potential_count][1] = j;
                potential_count++;
            }
            else if (weightGrid[i][j] == maxWeigth)
            {
                potential_cells[potential_count][0] = i;
                potential_cells[potential_count][1] = j;
                potential_count++;
            }
        }
    }

    srand(time(NULL));
    int random_index = rand() % potential_count;
    *row = potential_cells[random_index][0];
    *col = potential_cells[random_index][1];
}

void find_best_2by2_area_hard(HardBot hardBot, char PlayerGrid[GRID_SIZE][GRID_SIZE], int *row, int *col)
{
    int max_prob = 0;

    // the following array can fit 25 grids stored with their top_left coordinate
    int potential_areas[25][2];
    int potential_count = 0;

    for (int r = 0; r < 9; r++)
    {
        for (int c = 0; c < 9; c++)
        {

            if (!isCoordFoundInArray(hardBot.base.radaredCoord, r, c))
            {

                int prob = 0;

                for (int r1 = r; r1 < r + 2; r1++)
                {
                    for (int c1 = c; c1 < c + 2; c1++)
                    {
                        prob += hardBot.weightGrid[r1][c1];
                    }
                }

                if (prob > max_prob)
                {
                    max_prob = prob;
                    potential_count = 0;
                    potential_areas[potential_count][0] = r;
                    potential_areas[potential_count][1] = c;
                    potential_count++;
                }
                else if (prob == max_prob)
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

void find_best_torpedo_rc(HardBot hardBot, char PlayerGrid[GRID_SIZE][GRID_SIZE], int *choice, int *value)
{
    int maxWeightRow = 0;
    int maxWeightCol = 0;
    int bestRow = 0;
    int bestCol = 0;
    for (int i = 0; i < GRID_SIZE; i++)
    {
        int weightRow = 0;
        int weightCol = 0;

        for (int j = 0; j < GRID_SIZE; j++)
        {
            weightRow += hardBot.weightGrid[i][j];
            weightCol += hardBot.weightGrid[j][i];

            if (weightRow > maxWeightRow)
            {
                maxWeightRow = weightRow;
                bestRow = i;
            }

            if (weightCol > maxWeightCol)
            {
                maxWeightCol = weightCol;
                bestCol = j;
            }
        }

        if (maxWeightRow >= maxWeightCol)
        {
            *choice = 1;
            *value = bestRow;
        }

        else
        {
            *choice = 2;
            *value = bestCol;
        }
    }
}

void coordToSmoke(HardBot *hardBot, int *row, int *col)
{

    for (int i = 0; i < GRID_SIZE - 1; i++)
    {
        for (int j = 0; j < GRID_SIZE - 1; j++)
        {
            char shipInitial = hardBot->base.base.base.own.hide[i][j];
            if (shipInitial != '~')
            {
                int shipNum;

                if (shipInitial == 'c')
                    shipNum = 0;
                if (shipInitial == 'b')
                    shipNum = 1;
                if (shipInitial == 'd')
                    shipNum = 2;
                if (shipInitial == 's')
                    shipNum = 3;

                if (hardBot->base.base.base.ships[shipNum].hitCount == 0 &&
                    !(hardBot->smokedShips[shipNum]))

                {
                    *row = i;
                    *col = j;
                    hardBot->smokedShips[shipNum] = true;
                    return;
                }
            }
        }
    }
}

bool shouldUseSmoke(HardBot hardBot)
{
    bool useSmoke = true;
    int count = 0;
    for (int i = 0; i < 4; i++)
    {
        if (hardBot.base.base.base.ships[i].hitCount > 0)
            count++;
    }
    if (count == 4)
        useSmoke = false;
    return useSmoke && hardBot.base.base.base.smokeUses > 0;
}

void generate_hard_bot_move(HardBot *hardBot, char PlayerGrid[10][10], char move[9], int *row, int *col)
{

    if (hardBot->base.base.hit_stack_size == 0)
        hardBot->base.base.haunting = true;
    // if the medium bot is not haunting

    // check if we should use Torpedo
    if (hardBot->base.base.base.Torpedo)
    {
        strcpy(move, "Torpedo");
        find_best_torpedo_rc(*hardBot, PlayerGrid, row, col);
        hardBot->base.base.last_move = 4;
    }

    else if (hardBot->base.base.base.Artillery)

    {
        find_best_2by2_area_hard(*hardBot, PlayerGrid, row, col);
        strcpy(move, "Artillery");
        hardBot->base.base.last_move = 3;
    }

    else if (!hardBot->base.base.haunting)
    {
        // we set the move to be fire
        strcpy(move, "Fire");
        // we concatentate to the move the coordinates found last in the hit stack
        // we decrement the hit stack size
        --hardBot->base.base.hit_stack_size;
        *row = hardBot->base.base.hit_stack[hardBot->base.base.hit_stack_size][0];
        *col = hardBot->base.base.hit_stack[hardBot->base.base.hit_stack_size][1];
    }
    else if (shouldUseSmoke(*hardBot))
    {
        strcpy(move, "Smoke");
        hardBot->smokeUsed++;
        coordToSmoke(hardBot, row, col);
        hardBot->base.base.last_move = 5;
    }

    // else if we are haunting for places to target
    // and we can still use radar and it is a good choice to make use of it in this turn
    else if (should_use_radar(&(hardBot->base), PlayerGrid))
    {

        // finds the best coordiates to tagret by radar and stores it in radar target
        find_best_2by2_area_hard(*hardBot, PlayerGrid, row, col);
        // we decrement the number of radar sweeps we can still use
        strcpy(move, "Radar");
        hardBot->base.base.last_move = 2;
        hardBot->base.ConsecutiveMisses = 0;
    }
    // if we're haunting but it's not a good choice to use radar
    else
    {

        strcpy(move, "Fire");
        hardBot->base.base.last_move = 1;
        selectTarget(row, col, hardBot->weightGrid);
    }
}

void printIntGrid(int grid[10][10])
{

    printf("  A B C D E F G H I J \n");

    for (int row = 0; row < 10; row++)
    {
        printf("%d ", row + 1);
        for (int col = 0; col < 10; col++)
        {

            printf("%d ", grid[row][col]);
        }

        printf("\n");
    }
}

void filterHitStack(char playerGrid[GRID_SIZE][GRID_SIZE], int hitStack[100][2], int *stackSize)
{
    int filteredIndex = 0;

    for (int i = 0; i < *stackSize; i++)
    {
        int x = hitStack[i][0];
        int y = hitStack[i][1];

        // Check if the element in playerGrid at the given coordinates is '*'
        if (playerGrid[x][y] == '*')
        {
            hitStack[filteredIndex][0] = x;
            hitStack[filteredIndex][1] = y;
            filteredIndex++;
        }
    }

    // Update the size of the stack to the number of valid elements
    *stackSize = filteredIndex;
}

int main()
{
    Ship shipss[] = {{"carrier", 5, 0}, {"battleship", 4, 0}, {"destroyer", 3, 0}, {"submarine", 2, 0}};

    char name[50];
    printf("Please insert your name: ");
    scanf("%s", &name);

    Player player;

    player.name = name;
    player.radarUses = 3;
    player.smokeUses = 0;
    player.countSunk = 0;
    player.Torpedo = false;
    player.Artillery = false;
    player.isTurn = 1;
    Grid PlayerGrid;
    player.own = PlayerGrid;

    printf("%s welcome to World War III \"Battleship!\"\n", player.name);

    do
    { // asking players to select the bot modes
        printf("Please select difficulty level of the bot: insert 0 for easy and 1 for medium and 2 for hard: ");
        scanf("%d", &difficulty);
    } while (difficulty != 0 && difficulty != 1 && difficulty != 2);

    Player *bot;
    MediumBot medium;
    EasyBot easy;
    HardBot hard;

    if (difficulty == 0)
    {
        bot = (Player *)&easy;
        easy.haunting = true;
        easy.orientation = 0;
        easy.hit_stack_size = 0;
        easy.last_move = 1;
        easy.troubleCells[0][0] = 0;
        easy.troubleCells[0][1] = 0;
        easy.troubleCells[1][0] = 0;
        easy.troubleCells[1][1] = 0;
    }
    else if (difficulty == 1)
    {
        bot = (Player *)&(medium.base);

        medium.base.haunting = true;
        medium.base.orientation = 0;
        medium.base.hit_stack_size = 0;
        medium.base.last_move = 1;
        medium.base.troubleCells[0][0] = 0;
        medium.base.troubleCells[0][1] = 0;
        medium.base.troubleCells[1][0] = 0;
        medium.base.troubleCells[1][1] = 0;

        medium.ConsecutiveMisses = 0;

        for (int i = 0; i < 5; i++)
        {

            medium.radaredCoord[i][0] = 0;
            medium.radaredCoord[i][1] = 0;
        }
    }
    else
    {

        hard.base.base.haunting = true;
        hard.base.base.orientation = 0;
        hard.base.base.hit_stack_size = 0;
        hard.base.base.last_move = 1;
        hard.base.base.troubleCells[0][0] = 0;
        hard.base.base.troubleCells[0][1] = 0;
        hard.base.base.troubleCells[1][0] = 0;
        hard.base.base.troubleCells[1][1] = 0;

        hard.base.ConsecutiveMisses = 0;

        bot = (Player *)&(hard.base.base);
        hard.smokeUsed = 0;
        for (int i = 0; i < 5; i++)
        {
            hard.smokedShips[i] = false;

            hard.base.radaredCoord[i][0] = 0;
            hard.base.radaredCoord[i][1] = 0;
        }
    }

    bot->name = "Bot";
    bot->radarUses = 3;
    bot->smokeUses = 0;
    bot->countSunk = 0;
    bot->Torpedo = false;
    bot->Artillery = false;
    bot->isTurn = 0;
    Grid BotGrid;
    bot->own = BotGrid;

    for (int i = 0; i < 4; i++)
    {
        player.ships[i] = shipss[i];
        bot->ships[i] = shipss[i];
    }

    // initializing each player's grid (both the displayed and the hidsden) to have all water(~)
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            player.own.display[i][j] = '~';
            player.own.hide[i][j] = '~';

            bot->own.hide[i][j] = '~';
            bot->own.display[i][j] = '~';

            player.smoked[i][j] = false;
            bot->smoked[i][j] = false;

            if (difficulty == 2)
                hard.weightGrid[i][j] = 0;
        }
    }

    printf("%s start filling your grid\n", player.name);
    positionShips(&player, shipss);

    printf("The bot is palcing its ships...\n");
    sleepp(3);

    placeBotShipsRandomly(bot, shipss);

    printf("%s are you ready for the battle?\nLet's go!\n", player.name);

    int gameOver = 0;

    while (!gameOver)
    {

        char move[9];
        char coord[4];
        int row;
        int col;
        bool hit;

        if (player.isTurn)
        {
            // Display updated opponent(defender) grid before the turn
            printf("Updated grid for bot\n");
            printGrid(bot->own.display);

            // commanding the attacker to choose their move
            printf("%s, choose your move Fire/Radar/Smoke/Artillery/Torpedo: ", player.name);
            scanf("%s", move);

            if (strcmp(move, "Fire") && strcmp(move, "Radar") && strcmp(move, "Smoke") && strcmp(move, "Artillery") && strcmp(move, "Torpedo"))
            {
                printf("Invalid move! you loose your turn.\n");
                switchTurns(&player, bot);
                continue; // Skip this turn and ask again
            }

            if (!strcmp(move, "Torpedo"))
            {
                // row is choice (1 if row, 2 if col) and col is the value of row or col
                char c;

                printf("Row(1) or Column(2)?\n");
                scanf("%d", &row);

                if (row == 1)
                {

                    printf("choose a row from 1 to 10: ");
                    scanf("%d", &col);
                    col--;
                }
                else
                {

                    printf("choose a column from A to J: ");
                    scanf(" %c", &c);
                    col = letterToNumber(c);
                }
            }

            else
            {
                scanf("%s", coord);
                extract_row_col(coord, &row, &col);

                if (!validateCoordinates(row, col))
                {
                    printf("Invalid coordinates, you lost your turn! \n");
                    switchTurns(&player, bot);
                    continue;
                }
            }

            hit = takeTurn(&player, bot, move, row, col); // player attacks bot
        }
        else
        {

            // Display updated opponent(defender) grid before the turn
            printf("Updated grid for %s:\n", player.name);
            printGrid(player.own.display);

            int shipsunkprev = bot->countSunk;
            if (difficulty == 0)
            {
                generate_easy_bot_move(&easy, player.own.display, &row, &col);
                strcpy(move, "Fire");
            }
            else if (difficulty == 1)
            {

                generate_medium_bot_move(&medium, player.own.display, move, &row, &col);
            }

            else
            {
                generate_hard_bot_move(&hard, player.own.display, move, &row, &col);
            }
            if (strcmp(move, "Smoke"))
            {
                if (!strcmp(move, "Torpedo"))
                    printf("bot used Torpedo, choice: %d, line: %d\n", row, col + 1);
                else
                    printf("bot made a move %s at %c%d\n", move, 'A' + col, row + 1);
            }

            hit = takeTurn(bot, &player, move, row, col);

            if (difficulty == 0)
            {

                if (hit)
                {
                    update_bot_state_after_hit(&easy, row, col, player.own.display);
                    easy.haunting = false;
                    if (shipsunkprev < bot->countSunk)
                    {
                        // instead of easy there was medium.base becauseit was copied from medium
                        easy.orientation = 0;
                        filterHitStack(player.own.display, easy.hit_stack, &(easy.hit_stack_size));
                        if (player.own.display[easy.troubleCells[0][0]][easy.troubleCells[0][1]] == '*')
                        {
                            check_around_hit(&easy, player.own.display, easy.troubleCells[0][0], easy.troubleCells[0][1]);
                        }
                        else if (player.own.display[easy.troubleCells[1][0]][easy.troubleCells[1][1]] == '*')
                        {
                            check_around_hit(&easy, player.own.display, easy.troubleCells[1][0], easy.troubleCells[1][1]);
                        }
                    }
                }
            }
            else if (difficulty == 1)
            {
                if (hit)
                {
                    medium.ConsecutiveMisses = 0;
                    update_bot_state_after_hit(&(medium.base), row, col, player.own.display);

                    if (shipsunkprev < bot->countSunk)
                    {
                        medium.base.orientation = 0;
                        filterHitStack(player.own.display, medium.base.hit_stack, &(medium.base.hit_stack_size));

                        if (player.own.display[medium.base.troubleCells[0][0]][medium.base.troubleCells[0][1]] == '*')
                        {
                            check_around_hit(&(medium.base), player.own.display, medium.base.troubleCells[0][0], medium.base.troubleCells[0][1]);
                        }
                        else if (player.own.display[medium.base.troubleCells[1][0]][medium.base.troubleCells[1][1]] == '*')
                        {
                            check_around_hit(&(medium.base), player.own.display, medium.base.troubleCells[1][0], medium.base.troubleCells[1][1]);
                        }
                    }
                }
                else
                {
                    medium.ConsecutiveMisses++;
                }
            }
            else
            {

                if (!strcmp(move, "Fire") || !strcmp(move, "Artillery") || !strcmp(move, "Torpedo"))
                {
                    for (int i = 0; i < 10; i++)
                    {
                        for (int j = 0; j < 10; j++)
                        {
                            hard.weightGrid[i][j] = 0;
                        }
                    }
                    calculateWeight(hard.weightGrid, player);
                }
                if (hit)
                {
                    hard.base.ConsecutiveMisses = 0;
                    update_bot_state_after_hit(&(hard.base.base), row, col, player.own.display);

                    if (shipsunkprev < bot->countSunk)
                    {
                        hard.base.base.orientation = 0;
                        filterHitStack(player.own.display, hard.base.base.hit_stack, &(hard.base.base.hit_stack_size));

                        if (player.own.display[hard.base.base.troubleCells[0][0]][hard.base.base.troubleCells[0][1]] == '*')
                        {
                            check_around_hit(&(hard.base.base), player.own.display, hard.base.base.troubleCells[0][0], hard.base.base.troubleCells[0][1]);
                        }
                        else if (player.own.display[hard.base.base.troubleCells[1][0]][hard.base.base.troubleCells[1][1]] == '*')
                        {
                            check_around_hit(&(hard.base.base), player.own.display, hard.base.base.troubleCells[1][0], hard.base.base.troubleCells[1][1]);
                        }
                    }
                }
                else
                {
                    hard.base.ConsecutiveMisses++;
                }
            }
        }

        if (player.countSunk == 4)
        {
            printf("%s wins!\n", player.name);

            gameOver = 1;
        }
        else if (bot->countSunk == 4)
        {
            printf("%s", "bot wins!\n");
            printf("Winner's hidden ships\n");
            printGrid(bot->own.hide);
            gameOver = 1;
        }

        // Switch turns
        switchTurns(&player, bot);
    }

    char exit;
    printf("Press any key to exsit!\n");
    scanf("%c", &exit);

    return 0;
}
