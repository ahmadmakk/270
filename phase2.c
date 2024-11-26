// This project was done by: Nermine Chhayta, Nour Fneish, Ahmad Makkouk and Hayat Bounite
// Team: World war III
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

// letterToNumber(A) returns 0 and letterToNumber(B) returns 1 ...
// this function will later help us in manipulating the columns as they are represented by letters in our grids
int letterToNumber(char letter)
{
    return letter - 'A';
}

// check whether a ship can be deployed by the player in the position specified
// returns 0 if the position if the placement can't occur

// we can add print statements to inform the player of failed positions
int check(char position[], Player *player, int cells, char ship)
{

    //  Convert letter of colomn to index
    int column = letterToNumber(position[0]);

    // here we just decremt 1 from the row to make it aligned with 0 indexing
    int row;
    if (position[1] == '1' && position[2] == '0')
    {            // Checking for "10"
        row = 9; // Since the grid is zero-indexed, row 10 becomes 9
    }
    else if (position[1] == '1' && position[2] != 0 && position[2] != ',')
        row = 20;
    else
    {
        row = position[1] - '1'; // For single-digit rows
    }

    // we ignore position[2] since it has a ,
    //  store the direction in a char
    char direction = position[3];

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
            printf("Position provided out of the grid!\n");
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
            printf("Position provided out of the grid!\n");
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

// Function to switch the turn between the two players
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
int isShipSunk(Ship *ship)
{
    return ship->hitCount == ship->cells;
}

bool Radar(Player *attacker, Player *opponent, int topLeftRow, int topLeftCol)
{
    bool hit = false;
    // Disable Torpedo and Artillery abilities
    attacker->Torpedo = false;
    attacker->Artillery = false;

    // Check if there are any radar scans left
    if (attacker->radarUses <= 0)
    {
        printf("No radar scans left. Your turn is skipped.\n");
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
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            // Skip cells covered by smoke screens
            if (opponent->smoked[topLeftRow + i][topLeftCol + j])
            {
                continue;
            }

            char currentCell = opponent->own.hide[topLeftRow + i][topLeftCol + j];
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
        hit = true;
    }
    else
    {
        printf("No enemy ships found in the area.\n");
    }

    // Decrement radar use count
    attacker->radarUses--;

    return hit;
}
void Smoke(Player *attacker, int topLeftRow, int topLeftCol)
{
    // Disable artillery when using a smoke screen
    attacker->Artillery = false;
    attacker->Torpedo = false;

    // Validate if the 2x2 area is within grid bounds
    if (!validateCoordinates(topLeftRow + 1, topLeftCol + 1))
    {
        printf("Invalid smoke screen area! You lose your turn.\n");
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

    // Decrement smoke screen count after successful placement
    attacker->smokeUses--;

    // Clear screen for next player's turn (optional, mainly for console-based games)
   // system("cls");

    printf("Smoke screen placed successfully!\n");
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
                opponent->own.display[row][col] = '*';
                hit = true; // Mark that a hit occurred
                for (int i = 0; i < 4; i++)
                {
                    if (cellInHide == opponent->ships[i].name[0]) // Check if the hit matches a ship's initial
                    {
                        opponent->ships[i].hitCount++; // Increment hit count for the ship

                        if (isShipSunk(&opponent->ships[i])) // Check if the ship is sunk
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

// choice: 1 for row 2 for column
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
            if (cell != '~' && cell != '*')
            { // Ship detected and not hit before
                hit = true;
                opponent->own.display[line][j] = '*';
                opponent->own.hide[line][j] = '*';

                // Increment the hit count for the specific ship
                for (int i = 0; i < 4; i++)
                {
                    if (cell == opponent->ships[i].name[0])
                    { // Match ship by initial
                        opponent->ships[i].hitCount++;

                        // Check if the ship is sunk
                        if (isShipSunk(&opponent->ships[i]))
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
            if (cell != '~' && cell != '*')
            { // Ship detected and not hit before
                hit = true;
                opponent->own.display[j][line] = '*';
                opponent->own.hide[j][line] = '*';

                // Increment the hit count for the specific ship
                for (int i = 0; i < 4; i++)
                {
                    if (cell == opponent->ships[i].name[0])
                    { // Match ship by initial
                        opponent->ships[i].hitCount++;

                        // Check if the ship is sunk
                        if (isShipSunk(&opponent->ships[i]))
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

bool fire(Player *attacker, Player *defender, int row, int column)
{
    bool hit = false;

    int sunkshipprev = attacker->countSunk;

    char cellinHide = defender->own.hide[row][column];
    char cellinDisplay = defender->own.display[row][column];

    if (cellinDisplay == '*')
    {
        printf("The cell is already hit!");
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

                if (isShipSunk(&defender->ships[i])) // Check if the ship is sunk
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

bool takeTurn(Player *attacker, Player *defender, char move[9], int row, int col)
{
    bool hit = false;

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
    else if (!strcmp("Smoke", move))
    {
        printf("Torpedo selected!\n");
        hit = torpedoStrike(attacker, defender, row, col);
    }
    else
    {
        printf("Invalid move!\n");
    }

    // Display updated opponent(defender) grid after the turn
    printf("Updated grid for %s:\n", defender->name);
    printGrid(defender->own.display);

    sleep(5);
   // system("cls");
    return hit;
}

void positionShips(Player *player, Ship *shipss)
{
    // declaring position array for ships
    /*expected input for position: letter of column then number of row then comma followed by the direction
       e.g: A1,horizantal
       we take the size 5 because we can take only the first letter from the direction 'h' or 'v'
       */
    char position[5];

    // first we print their grid (which's initially all water)
    printGrid(player->own.display);

    // a for loop to pass over all the ships in the ships array
    for (int i = 0; i < 4; i++)
    {
        // asking the player for the position and storing it
        printf("Please enter the position of the %s: ", shipss[i].name);
        scanf("%s", &position);

        // checking if the position is valid with our check function
        if (!check(position, player, shipss[i].cells, shipss[i].name[0]))
        {
            //  printf("Invalid coordinates\n"); // should be informed in the function check
            i--; // decremnting i so we pass over the ship again and the player get to deploy it correclty
        }

        // after each placement the player is shown their grid
        printGrid(player->own.hide);
    }
}

// game end logic
int isGameOver(Player *player)
{
    return player->countSunk == 4;
}
// display the winner mssg
void displayWinner(Player *winner)
{
    printf("congratulations! %s YOU WON!", winner->name);
}

void placeBotShipsRandomly(Player *player, Ship ships[])
{
    // Initialize random seed
    srand(time(NULL));

    // Iterate over all ships to place them on the grid
    for (int i = 0; i < 4; i++)
    {
        Ship currentShip = ships[i];
        int placed = 0; // Flag to track if the ship was successfully placed

        while (!placed)
        {
            // Generate random starting position and direction
            int row = rand() % 10;                   // Random row (0 to 9)
            int col = rand() % 10;                   // Random column (0 to 9)
            char direction = rand() % 2 ? 'h' : 'v'; // Random direction ('h' or 'v')

            // Validate placement
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
    else if (coord[1] == '1' && coord[2] != 0)
        *row = 20;
    else
    {
        *row = coord[1] - '1'; // For single-digit rows
    }
}

typedef struct
{

    Player base;

    bool haunting;          // if 1 then we need to haunt for targets if 0 then we have targets
    int last_move;          // if 1, the last hit was fire, 2 for radar, 3 for artillery
    int hit_stack[100][2]; // Stack of coordinates to target next
    int hit_stack_size;     // Size of the hit stack
    int ConsecutiveMisses;  /*tracks consecutive misses to see if the bot is struggling
                              should be 0-ed in the main if the bot hits */
    int orientation;        // if 1 then vertical, if 2 then horizontal, if 0 then we're not even hitting

} MediumBot;

bool should_use_radar(MediumBot *MediumBot, char PlayerGrid[10][10])
{

    // count how many unkown cells (~) there are
    int unknown_cells = 0;
    for (int row = 0; row < 10; row++)
    {
        for (int col = 0; col < 10; col++)
        {
            if (PlayerGrid[row][col] == '~')
            {
                unknown_cells++;
            }
        }
    }

    if (unknown_cells < 50 && MediumBot->ConsecutiveMisses > 3)
    {
        return 1; // Mid-game and struggling to find targets -> use radar
    }
    else if (unknown_cells < 25)
    {
        return 1; // Late game, prioritize finding remaining ships
    }

    return 0; // Avoid radar in the early game (if unkown cells are many  >50)
}

void medium_guess(char PlayerGrid[10][10], int* row, int* col)
{

    for (int r = 0; r < 10; r++)
    {
        for (int c= 0; c < 10; c++)
        {
            if ((r + c) % 2 == 0 && PlayerGrid[r][c] == '~')
            {
 
                 *col= c;             
                 *row = r;
                return;
            }
        }
    }
}

// used for radar and artillery
void find_best_2by2_area(char PlayerGrid[10][10],  int* row, int* col)
{

    int max_water = -1;
    int best_row = -1;
    int best_col = -1;


    for (int r = 0; r < 9; r++)
    {
        for (int c = 0; c < 9; c++)
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

                if (water > max_water)
                {
                    max_water = water;
                    best_col = c;
                    best_row = r;
                }


                if (max_water == 4)
                    break; // we can't reach more than 4 per 2x2 grid
            }
            if (max_water == 4)
                break;
        }

              if (max_water == 4)
                break;
    }


    *row = best_row;
    *col = best_col;
}

void addto_hit_stack(MediumBot *MediumBot, int row, int col, char PlayerGrid[10][10]) {
  
    if (validateCoordinates(row, col) && PlayerGrid[row][col] == '~') {
        MediumBot->hit_stack[MediumBot->hit_stack_size][0] = row;
        MediumBot->hit_stack[MediumBot->hit_stack_size][1] = col;
        MediumBot->hit_stack_size++;
    }

}


void check_around_hit(MediumBot *MediumBot, char PlayerGrid[10][10], int row, int col)
{

    int down_Up_left_right[4][2] = {{row - 1, col}, {row + 1, col}, {row, col - 1}, {row, col + 1}};

    if (MediumBot->orientation == 0)
    {
        if (validateCoordinates(row + 1, col) && PlayerGrid[row + 1][col] == '*')
            MediumBot->orientation = 1;
        else if (validateCoordinates(row - 1, col) && PlayerGrid[row - 1][col] == '*')
            MediumBot->orientation = 1;

        else if (validateCoordinates(row, col + 1) && PlayerGrid[row][col + 1] == '*')
            MediumBot->orientation = 2;
        else if (validateCoordinates(row, col - 1) && PlayerGrid[row][col - 1] == '*')
            MediumBot->orientation = 2;
    }

    // no orientation
    if (MediumBot->orientation == 0)
    {
        addto_hit_stack(MediumBot,  row+1, col, PlayerGrid);
        addto_hit_stack(MediumBot, row-1, col, PlayerGrid);
        addto_hit_stack(MediumBot, row, col-1, PlayerGrid);
        addto_hit_stack(MediumBot, row, col+1, PlayerGrid);
    }
    // vertical orientation
    if (MediumBot->orientation == 1)
    {
        if (validateCoordinates(row + 1, col) && PlayerGrid[row + 1][col] == '*')
            addto_hit_stack(MediumBot, row-1, col, PlayerGrid);
        else
            addto_hit_stack(MediumBot, row+1, col,  PlayerGrid);
    }
    // horizontal orienatation
    if (MediumBot->orientation == 2)
    {
        if (validateCoordinates(row, col + 1) && PlayerGrid[row][col + 1] == '*')
            addto_hit_stack(MediumBot, row, col-1, PlayerGrid);
        else
            addto_hit_stack(MediumBot, row, col+1, PlayerGrid);
    }
}

void generate_medium_bot_move(MediumBot *MediumBot, char PlayerGrid[10][10], char move[9], int* row, int* col)
{

    if (MediumBot->hit_stack_size == 0)
        MediumBot->haunting = 1;
    // if the medium bot is not haunting

    if (MediumBot->base.Artillery)

    {
        find_best_2by2_area(PlayerGrid, row, col);
        strcpy(move, "Artillery");
        MediumBot->last_move = 3;
    }

    else if (!MediumBot->haunting)
    {
        // we set the move to be fire
        strcpy(move, "Fire");
        // we concatentate to the move the coordinates found last in the hit stack
        // we decrement the hit stack size
        --MediumBot->hit_stack_size;
        *row = MediumBot-> hit_stack[MediumBot->hit_stack_size][0];
        *col =  MediumBot-> hit_stack[MediumBot->hit_stack_size][1];
    }

    // else if we are haunting for places to target
    // and we can still use radar and it is a good choice to make use of it in this turn
    else if (MediumBot->base.radarUses > 0 && should_use_radar(MediumBot, PlayerGrid))
    {

        // finds the best coordiates to tagret by radar and stores it in radar target
        find_best_2by2_area(PlayerGrid, row, col);
        // we decrement the number of radar sweeps we can still use
        //  move  = Radar + radar target
        strcpy(move, "Radar");
        MediumBot->last_move = 2;
    }
    // if we're haunting but it's not a good choice to use radar
    else
    {
        // we'll have to make a guess but not a completely random one
        medium_guess(PlayerGrid, row, col);
        strcpy(move, "Fire");
        MediumBot->last_move = 1;
    }
}

// the following function populates the hit stack with the possible adjacent targets of
// a hit once there is one using fire (up left, )
// radar returns "true" meaning Enemy ships found in the area so we store them in the stack
// currently radar returns void so we should make it return bool for that
// artilley hits
void update_bot_state_after_hit(MediumBot *MediumBot,int row, int col, char PlayerGrid[10][10])
{

    MediumBot->haunting = 0;
    

    // if last move was fire we check adjacent cells depending on orientation
    if (MediumBot->last_move = 1)
    {
        check_around_hit(MediumBot, PlayerGrid, row, col);
    }
    // if last move was radar we check the 2X2 grid of which hit_coord is the top left corner

    else if (MediumBot->last_move = 2)
    {

        int candidates[4][2] = {{row, col}, {row + 1, col}, {row, col + 1}, {row + 1, col + 1}};

        for (int i = 0; i < 4; i++)
        {
            addto_hit_stack(MediumBot, row, col, PlayerGrid);
        }
    }

    // if last move was artilery we check eaxh cell in the 2X2 grid of which hit_coord is the top left corner
    // if each cell was a hit or not

    else if (MediumBot->last_move = 3)
    {

        int candidates[4][2] = {{row, col}, {row + 1, col}, {row, col + 1}, {row + 1, col + 1}};

        for (int i = 0; i < 4; i++)
        {

            int r = candidates[i][0];
            int c = candidates[i][1];

            if (PlayerGrid[r][c] == '*')
            {
                check_around_hit(MediumBot, PlayerGrid, r, c);
            }
        }
    }

}


void print2D(int array[100][2])
{

    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            printf("%d%d", array[i][j]);
        }
         printf("\n");
    }

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




    // asking players to select the bot modes
    printf("Please select difficulty level of the bot: insert 0 for easy and 1 for medium and 2 for hard: ");
    scanf("%d", &difficulty);


    Player *  bot;
    MediumBot medium;


    if (difficulty == 0)
    {
    }
    else if (difficulty == 1)
    {
        bot = (Player *) &medium;
        medium.ConsecutiveMisses = 0;
        medium.orientation = 0;
        medium.hit_stack_size = 0;
        medium.haunting = 1;
    }
    else
    {
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
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            player.own.display[i][j] = '~';
            player.own.hide[i][j] = '~';
            bot->own.hide[i][j] = '~';
            bot->own.display[i][j] = '~';
        }
    }

    printf("%s start filling your grid\n", player.name);
    positionShips(&player, shipss);
  //  system("cls");

    printf("The bot is palcing its ships...\n");
    //sleep(5);

    if (difficulty ==0 || difficulty ==1){
    placeBotShipsRandomly(bot, shipss);
    }
    else {
        // call hayat's function of smart placement of ships 
    }

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

                extract_row_col(coord, &row, &col);
                if (!validateCoordinates(row, col))
                {
                    printf("Invalid coordinates, you lost your turn!");
                    break;
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

            }
            else if (difficulty == 1)
            {
               
                generate_medium_bot_move(&medium, player.own.display, move, &row, &col);
                 printf("bot made a move at %c%d\n", 'A'+col ,row+1 );
            }

            else
            {

            }

            hit = takeTurn(bot, &player, move, row, col);

            if (difficulty == 0)
            {


            }
            else if (difficulty == 1)
            {
                if (hit)
                {
                    if (shipsunkprev < bot->countSunk)
                    medium.orientation = 0;
                    medium.ConsecutiveMisses = 0;
                    if(shipsunkprev == bot->countSunk)
                    update_bot_state_after_hit(&medium, row, col, player.own.display);
                }
                else
                {
                    medium.ConsecutiveMisses++;
                }
            }
            else
            {



            }
        }

        // Check end condition (e.g., all ships sunk)
        // For simplicity, we'll assume that if a player has sunk 4 ships, they win
        if (player.countSunk == 4)
        {
            printf("%s wins!\n", player.name);
            gameOver = 1;
        }
        else if (bot->countSunk == 4)
        {
             printf("%s", "bot wins!");
            gameOver = 1;
        }
        // Switch turns


        switchTurns(&player, bot);
    }

    char exit;
    printf("Press any key to exsit!");
    scanf("%c", &exit);
    return 0;
}
