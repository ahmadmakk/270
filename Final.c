//This project was done by: Nermine Chhayta, Nour Fneish, Ahmad Makkouk and Hayat Bounite
//Team: World war III
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define GRID_SIZE 10
#include <stdbool.h>
#include <string.h>
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

void Radar(Player *attacker, Player *opponent)
{
    // Disable Torpedo and Artillery abilities
    attacker->Torpedo = false;
    attacker->Artillery = false;

    // Check if there are any radar scans left
    if (attacker->radarUses <= 0)
    {
        printf("No radar scans left. Your turn is skipped.\n");
        return;
    }

    // Prompt for coordinates for radar scan
    char coord[4];
    printf("%s, enter coordinates for radar scan (top-left of 2x2 area): ", attacker->name);
    scanf("%s", coord);

    int topLeftCol = letterToNumber(coord[0]);
    int topLeftRow;
    if (coord[1] == '1' && coord[2] == '0')
    {                   // Checking for "10"
        topLeftRow = 9; // Since the grid is zero-indexed, row 10 becomes 9
    }
    else if (coord[1] == '1' && coord[2] != 0)
        topLeftRow = 20;
    else
    {
        topLeftRow = coord[1] - '1'; // For single-digit rows
    }

    // Validate if the 2x2 area is within grid bounds
    if (!validateCoordinates(topLeftRow, topLeftCol) ||
        !validateCoordinates(topLeftRow + 1, topLeftCol + 1))
    {
        printf("Invalid radar scan area! You lose your turn.\n");
        return;
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
    }
    else
    {
        printf("No enemy ships found in the area.\n");
    }

    // Decrement radar use count
    attacker->radarUses--;
}
void Smoke(Player *attacker)
{
    // Disable artillery when using a smoke screen
    attacker->Artillery = false;
    attacker->Torpedo = false;
    // Prompt for coordinates
    char coord[4];
    printf("%s, enter coordinates for smoke screen (top-left of 2x2 area): ", attacker->name);
    scanf("%s", coord);

    int topLeftCol = letterToNumber(coord[0]);
    int topLeftRow;
    if (coord[1] == '1' && coord[2] == '0')
    {                   // Checking for "10"
        topLeftRow = 9; // Since the grid is zero-indexed, row 10 becomes 9
    }
    else if (coord[1] == '1' && coord[2] != 0)
        topLeftRow = 20;
    else
    {
        topLeftRow = coord[1] - '1'; // For single-digit rows
    }

    // Validate if the 2x2 area is within grid bounds
    if (!validateCoordinates(topLeftRow, topLeftCol) ||
        !validateCoordinates(topLeftRow + 1, topLeftCol + 1))
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
    system("cls");

    printf("Smoke screen placed successfully!\n");
}

void artilleryAttack(Player *attacker, Player *opponent)
{
    if (!attacker->Artillery)
    {
        printf("Artillery move not unlocked!\n");
        return;
    }
    int sunkshipprev = attacker->countSunk;

    char coord[4];
    printf("%s, enter coordinates for artillery strike (top-left of 2x2 area): ", attacker->name);
    scanf("%s", coord);

    int topLeftCol = letterToNumber(coord[0]);
    int topLeftRow;
    if (coord[1] == '1' && coord[2] == '0')
    {                   // Checking for "10"
        topLeftRow = 9; // Since the grid is zero-indexed, row 10 becomes 9
    }
    else if (coord[1] == '1' && coord[2] != 0)
        topLeftRow = 20;
    else
    {
        topLeftRow = coord[1] - '1'; // For single-digit rows
    }
    // Validate if the 2x2 area is within grid bounds
    if (!validateCoordinates(topLeftRow, topLeftCol) ||
        !validateCoordinates(topLeftRow + 1, topLeftCol + 1))
    {
        printf("Invalid artillery target area! You lose your turn.\n");
        return;
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
                            printf("You sunk a %s!\n", opponent->ships[i].name); // Print which ship was sunk
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
            { // Water cell
                if (difficulty == 0)
                { // Easy mode
                    opponent->own.display[row][col] = 'o';
                }
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
}

void torpedoStrike(Player *attacker, Player *opponent)
{
    if (!attacker->Torpedo)
    {
        printf("Torpedo move not unlocked!\n");
        return;
    }

    int choice;
    int line;
    char c;

    printf("Row(1) or Column(2)?\n");
    scanf("%d", &choice);

    if (choice == 1)
    {

        printf("choose a row from 1 to 10: ");
        scanf("%d", &line);
        line--;
    }
    else
    {

        printf("choose a column from A to J: ");
        scanf(" %c", &c);
        line = letterToNumber(c);
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
}

void fire(Player *attacker, Player *defender)
{
    int sunkshipprev = attacker->countSunk;
    char coord[4];

    printf("%s, enter coordinates to fire : ", attacker->name);
    scanf("%s", coord);

    int column = letterToNumber(coord[0]);

    int row;
    if (coord[1] == '1' && coord[2] == '0')
    {            // Checking for "10"
        row = 9; // Since the grid is zero-indexed, row 10 becomes 9
    }
    else if (coord[1] == '1' && coord[2] != 0)
        row = 20;
    else
    {
        row = coord[1] - '1'; // For single-digit rows
    }

    if (validateCoordinates(row, column))
    {
        char cellinHide = defender->own.hide[row][column];
        char cellinDisplay = defender->own.display[row][column];

        if (cellinDisplay == '*')
        {
            printf("The cell is already hit!");
        }

        else if (cellinHide != '~')
        {
            printf("Hit!\n");
            defender->own.display[row][column] = '*'; // mark hit on the display grid

            for (int i = 0; i < 4; i++)
            {
                if (cellinHide == defender->ships[i].name[0]) // Check if the hit matches a ship's initial
                {
                    defender->ships[i].hitCount++; // Increment hit count for the ship

                    if (isShipSunk(&defender->ships[i])) // Check if the ship is sunk
                    {
                        printf("You sunk a %s!\n", defender->ships[i].name); // Print which ship was sunk
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
            if (difficulty == 0) // difficulty is easy
            {
                defender->own.display[row][column] = 'o'; // mark miss on the display grid only in easy mode
            }
        }
    }
    else
    {
        printf("Invalid coordinates! You lose your turn.\n");
    }

    if (sunkshipprev == attacker->countSunk)
    {
        attacker->Artillery = false;
        attacker->Torpedo = false;
    }
}

void takeTurn(Player *attacker, Player *defender, int difficulty)
{

    // Display updated opponent(defender) grid before the turn
    printf("Updated grid for %s:\n", defender->name);
    printGrid(defender->own.display);

    // declaring an array to store the move later inserted by the attacker player
    int move;

    // commanding the attacker to choose their move
    printf("%s, choose your move Fire(1)/Radar(2)/Smoke(3)/Artillery(4)/Torpedo(5): ", attacker->name);
    scanf("%d", &move);

    // the strcmp function compares the strings character by character and return 0 if equal

    // we examine all the movements possibilities
    if (move == 1)
    {
        fire(attacker, defender);
    }
    else if (move == 2)
    {

        printf("Radar Sweep selected!\n");
        Radar(attacker, defender);
    }
    else if (move == 3)
    {
        printf("Smoke Screen selected!\n");
        Smoke(attacker);
    }
    else if (move == 4)
    {
        printf("Artillery Attack selected!\n");
        artilleryAttack(attacker, defender);
    }
    else if (move == 5)
    {
        printf("Torpedo selected!\n");
        torpedoStrike(attacker, defender);
    }
    else
    {
        printf("Invalid move!\n");
    }

    // Display updated opponent(defender) grid after the turn
    printf("Updated grid for %s:\n", defender->name);
    printGrid(defender->own.display);

    printf("Your turn is over, press any letter and pass the device to %s\n", defender->name);
    char str[2];
    scanf("%s", &str);
    system("cls");
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



int main()
{

    // an array of Ships that has them all
    Ship shipss[] = {{"carrier", 5, 0}, {"battleship", 4, 0}, {"destroyer", 3, 0}, {"submarine", 2, 0}};

    // declaring the two players
    Player player1;
    Player player2;
    player1.radarUses = 3;
    player1.smokeUses = 0;
    player2.radarUses = 3;
    player2.smokeUses = 0;
    // declaring the two grids
    Grid grid1;
    Grid grid2;

    player1.countSunk = 0;
    player2.countSunk - 0;

    player1.Torpedo = false;
    player2.Torpedo = false;

    player1.Artillery = false;
    player2.Artillery = false;

    player1.own = grid1;
    player2.own = grid2;


    for (int i = 0; i < 4; i++)
    {
        player1.ships[i] = shipss[i];
        player2.ships[i] = shipss[i];
    }

    // initializing each player's grid (both the displayed and the hidden) to have all water(~)
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            player1.own.display[i][j] = '~';
            player2.own.display[i][j] = '~';
            player1.own.hide[i][j] = '~';
            player2.own.hide[i][j] = '~';
        }
    }

    // asking players to select easy or difficult modes
    printf("Please select difficulty level: insert 0 for easy and 1 for hard: ");
    scanf("%d", &difficulty);
    // integrated game difficulty selection for player customization
    ///////handleDificulty(&player1,&player2,difficulty);
    // declaring two char arrays to store the name of the players
    char name1[50];
    char name2[50];

    // reading player1 name
    printf("Player 1 please insert your name: ");
    scanf("%s", &name1);
    player1.name = name1;

    // reading player2 name
    printf("Player 2 please insert your name: ");
    scanf("%s", &name2);
    player2.name = name2;

    printf("%S & %S welcome to World War III \"Battleship!\"", player1.name, player2.name);

    // randomly choosing who will start first
    // refresh time to produce new random numbers
    srand(time(NULL));
    int randomPlayer = 1 + (rand() % 2);
    int whosturn = randomPlayer;

    // updating the isTurn of the players in the players structures
    if (whosturn == 1)
    {
        player1.isTurn = 1;
        player2.isTurn = 0;
    }
    else
    {
        player1.isTurn = 0;
        player2.isTurn = 1;
    }

    // if it was the turn of player one first
    if (player1.isTurn)
    {
        printf("%s will start filling their grid\n", player1.name);
        positionShips(&player1, shipss);
        // Clear screen for next player's turn (optional, mainly for console-based games)
        system("cls");
        printf("Now it's %s's turn to fill their grid\n", player2.name);
        positionShips(&player2, shipss);
        // Clear screen for next player's turn (optional, mainly for console-based games)
        system("cls");
    }

    else // if it was the turn of player two first
    {
        printf("%s will start filling their grid\n", player2.name);
        positionShips(&player2, shipss);
        // Clear screen for next player's turn (optional, mainly for console-based games)
        system("cls");
        printf("Now it's %s's turn to fill their grid\n", player1.name);
        positionShips(&player1, shipss);
        // Clear screen for next player's turn (optional, mainly for console-based games)
        system("cls");
    }

    // game starts
    if (player1.isTurn)
        printf("%s are you ready for the battle?\nLet's go!", player1.name);
    else
        printf("%s are you ready for the battle?\nLet's go!", player2.name);

    int gameOver = 0;
    while (!gameOver)
    {
        if (player1.isTurn)
        {
            takeTurn(&player1, &player2, difficulty); // player1 attacks player2
        }
        else
        {
            takeTurn(&player2, &player1, difficulty); // player2 attacks player1
        }

        // Check end condition (e.g., all ships sunk)
        // For simplicity, we'll assume that if a player has sunk 4 ships, they win
        if (player1.countSunk == 4)
        {
            printf("%s wins!\n", player2.name);
            gameOver = 1;
        }
        else if (player2.countSunk == 4)
        {
            printf("%s wins!\n", player1.name);
            gameOver = 1;
        }
        // Switch turns
        switchTurns(&player1, &player2);
    }
    char exit;
    printf("Press any key to exsit!");
    scanf("%c", &exit);
    return 0;
}
