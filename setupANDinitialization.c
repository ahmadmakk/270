#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define GRID_SIZE 10 
#include <stdbool.h>
/* display: to be attacked by the oppoenet player (~ or * or 0)
hide: to keep record of where player's ships are (letter of the ship or ~)*/
typedef struct grid
{
    char display[10][10];
    char hide[10][10];
} Grid;

typedef struct player
{
     bool smoked[GRID_SIZE][GRID_SIZE];
    char *name;
    int isTurn;
    int countSunk;
    int radarUses;
    int torpedoUses;
    int smokeUses;
    bool Artillery;
    Grid own;
    Ship ships[4];

} Player;

typedef struct ship
{
    char *name;
    int cells;
    int hitCount; // we need it to check later if the ship has been sunk or no
} Ship;

// a function to print a 2D array with letters in the first row and indices in the first colomn
void printGrid(char arr[10][10])
{

    printf("    A  B  C  D  E  F  G  H  I  J\n");

    for (int i = 0; i < 10; i++)
    {
        printf("%2d ", i + 1);
        for (int j = 0; j < 10; j++)
        {
            printf("%c ", arr[i][j]);
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
int check(char position[], char hide[10][10], int cells, char ship)
{

    //  Convert letter of colomn to index
    int column = letterToNumber(position[0]);

    // here we just decremt 1 from the row to make it aligned with 0 indexing
    int row;
    if (position[1] == '1' && position[2] == '0')
    {            // Checking for "10"
        row = 9; // Since the grid is zero-indexed, row 10 becomes 9
    }
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
        printf("Position provided out of the grid!");
        return 0; // unvalid position
    }

    // Check for placement based on direction

    if (direction == 'h')
    {

        // Check if placing horizontally would go out of bounds
        if (column + cells > 10)
        {
            printf("Position provided out of the grid!");
            return 0; // unvalid position
        }

        // Check if the cells are available
        for (int i = 0; i < cells; i++)
        {
            if (hide[row][column + i] != '~') // since '~' means available
            {
                printf("Cell is occupied!");
                return 0; // Cell is occupied so unvalid position
            }
        }

        // position is valid so we place it
        for (int i = 0; i < cells; i++)
        {
            hide[row][column + i] = ship; // Place the ship by adding its initial
        }
    }

    else if (direction == 'v')
    {
        if (row + cells > 10)
        {
            printf("Position provided out of the grid!");
            return 0;
        }
        for (int i = 0; i < cells; i++)
        {
            if (hide[row + i][column] != '~')
            {
                printf("Cell is occupied!");
                return 0;
            }
        }
        for (int i = 0; i < cells; i++)
        {
            hide[row + i][column] = ship;
        }
    }
    // direction is not vertical or horizontal!unvalid
    else
    {
        printf("Invalid direction!");
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

void takeTurn(Player *attacker, Player *defender, int difficulty)
{

    // declaring an array to store the move later inserted by the attacker player
    char move[10];

    // commanding the attacker to choose their move
    printf("%s, choose your move (Fire/Radar/Smoke/Artillery/Torpedo): ", attacker->name);
    scanf("%s", move);

    // the strcmp function compares the strings character by character and return 0 if equal

    // we examine all the movements possibilities
    if (!strcmp(move, "Fire"))
    {
        fire(attacker, defender, difficulty);
        printf("Radar Sweep selected!\n");
    }
    else if (!strcmp(move, "Radar"))
    {
        Radar(attacker, defender);
        printf("Radar Sweep selected!\n");
    }
    else if (!strcmp(move, "Smoke"))
    {
        Smoke(attacker, defender);
        printf("Smoke Screen selected!\n");
    }
    else if (!strcmp(move, "Artillery"))
    {
        artilleryAttack(attacker, defender);
        printf("Artillery Attack selected!\n");
    }
    else if (!strcmp(move, "Torpedo"))
    {
        torpedoStrike(attacker, defender);
        printf("Torpedo selected!\n");
    }
    else
    {
        printf("Invalid move!\n");
    }

    // Display updated opponent(defender) grid after the turn
    printf("Updated grid for %s:\n", defender->name);
    printGrid(defender->own.display);
}

void fire(Player *attacker, Player *defender, int difficulty)
{
    char coord[4];

    printf("%s, enter coordinates to fire : ", attacker->name);
    scanf("%s", coord);

    int column = letterToNumber(coord[0]);

    int row;
    if (coord[1] == '1' && coord[2] == '0')
    {            // Checking for "10"
        row = 9; // Since the grid is zero-indexed, row 10 becomes 9
    }
    else
    {
        row = coord[1] - '1'; // For single-digit rows
    }

    if (validateCoordinates(row, column))
    {
        char cellinHide = defender->own.hide[row][column];
        char cellinDisplay = defender->own.display[row][column];

        if (cellinDisplay == "*")
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
                    }
                    attacker->countSunk++;
                    break; // Stop checking once we find the correct ship
                }
            }
        }
        // else the cell was water in the hide grid and was not a ship
        else
        {
            printf("Miss!\n");
            if (!difficulty) // difficulty is easy
            {
                defender->own.display[row][column] = 'o'; // mark miss on the display grid only in easy mode
            }
        }
    }
    else
    {
        printf("Invalid coordinates! You lose your turn.\n");
    }
}

// Check if the row and column are valid
int validateCoordinates(int row, int column)
{
    return (row >= 0 && row < 10 && column >= 0 && column < 10);
}

int isShipSunk(Ship *ship)
{
    return ship->hitCount == ship->cells;
}

void positionShips(Player player, Ship *shipss)
{
    // declaring position array for ships
    /*expected input for position: letter of column then number of row then comma followed by the direction
    e.g: A1,horizantal
    we take the size 5 because we can take only the first letter from the direction 'h' or 'v'
    */
    char position[5];

    // first we print their grid (which's initially all water)
    printGrid(player.own.display);

    // a for loop to pass over all the ships in the ships array
    for (int i = 0; i < 4; i++)
    {
        // asking the player for the position and storing it
        printf("Please enter the position of the %s: ", shipss[i].name);
        scanf("%s", &position);

        // checking if the position is valid with our check function
        if (!check(position, player.own.hide, shipss[i].cells, shipss[i].name[0]))
        {
            printf("Invalid coordinates\n"); // should be informed in the function check
            i--;                             // decremnting i so we pass over the ship again and the player get to deploy it correclty
        }

        // after each placement the player is shown their grid
        printGrid(player.own.hide);
    }
}

int main()
{

    // an array of Ships that has them all
    Ship shipss[] = {{"carrier", 5, 0}, {"battleship", 4, 0}, {"destroyer", 3, 0}, {"submarine", 2, 0}};

    // declaring the two players
    Player player1;
    Player player2;

    // declaring the two grids
    Grid grid1;
    Grid grid2;

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
            grid1.display[i][j] = '~';
            grid2.display[i][j] = '~';
            grid1.hide[i][j] = '~';
            grid2.hide[i][j] = '~';
        }
    }

    // defining difficulty levels
    enum difficultyLevels
    {
        easy, // 0
        hard  // 1
    };
    int difficulty;

    // asking players to select easy or difficult modes
    printf("Please select difficulty level: insert 0 for easy and 1 for hard: ");
    scanf("%d", &difficulty);
    // integrated game difficulty selection for player customization
handleDificulty(&player1,&player2,difficulty);
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
        printf("%s will start filling his grid\n", player1.name);
        positionShips(player1, shipss);
        printf("Now it's %s's turn to fill his grid\n", player2.name);
        positionShips(player2, shipss);
    }

    else // if it was the turn of player two first
    {
        printf("%s will start filling his grid\n", player2.name);
        positionShips(player2, shipss);
        printf("Now it's %s's turn to fill his grid\n", player1.name);
        positionShips(player1, shipss);
    }

    // game starts
    if (player1.isTurn)
        printf("%s are you ready for the battle?\nLet's go!", player1.name);
    else
        printf("%s are you ready for the battle?\nLet's go!", player1.name);

    int gameOver = 0;

    while (!gameOver)
    {
        int x,y;
        if (player1.isTurn)
        {
            //Used validateMove function to check if player moves are valid before proceeding
            if(validateMove(&player1,x,y){
                takeTurn(&player1,&player2,difficulty);
            }
         
        else
        {
           switchTurns(&player1,&player2);
        }
        }
        //end conditions 
       // if (player1.countSunk == 4) {
       //     printf("%s wins!\n", player2.name);
        //    gameOver = 1;
       // } else if (player2.countSunk == 4) 
        //    printf("%s wins!\n", player1.name);
        //    gameOver = 1;
        //

        // Check end condition (e.g., all ships sunk)
        if(isGameOver(&player1)){
            displayWinner(&player2);
            gameOver=1;
        } else if(isGameOver(&player2)){
            displayWinner(&player1);
            gameOver=1;
        }
        
        

        // Switch turns
        switchTurns(&player1, &player2);
    }

    return 0;
}
//game end logic
int isGameOver(Player *player){
return player ->countSunk==4; 
}
//display the winner mssg
void displayWinner(Player *winner){
    printf("congratulations! %s YOU WON!", winner->name);
}
//the overall correctness of the game (including losing turn when invalid moves)
int validateMove(Player *attacker,int x,int y){
    if(x<0 ||x>=10 || y<0 ||y>=10 || attacker->own.display[x][y]!='~'){
printf("that's an invalid move! you lost your turn \n");
        return 0; //invalid move
    }
return 1; //valid move
}
//ensuring it works under both easy and hard modes
void handleDificulty(Player *p1, Player *p2, int mode){
    if(mode==0){//easy
printf("Easy mode on!\n");
    }
else if(mode==1){
printf("hard mode on! \n");
}







void fire(char coordinate[], Player *attacker, Player *opponent) {
    int column = letterToNumber(coordinate[0]); //nermine function.
    int row = coordinate[1] - '1'; 

   //check if he is shooting on the grid 
    if (row < 0 || row >= 10 || column < 0 || column >= 10) {
        printf("Invalid coordinates!\n");
        return;
    }

    // Check if ship is where he fired
    if ((*opponent).own.hide[row][column] != '~') {
        printf("Hit! You hit %s's ship at %s!\n", (*opponent).name, coordinate);
        // Update grid 
        (*opponent).own.display[row][column] = '*'; // hit
        (*opponent).own.hide[row][column] = '*';    
    } else {
        printf("Miss! No ship at this location.\n");
        (*opponent).own.display[row][column] = 'O'; //miss
    }

    //Print grid of opponent
    printf("%s's grid after the attack:\n", (*attacker).name);
    printGrid((*opponent).own.display);
    //the grid here is actually changed not just an instance.
}

void radar_sweep(Player *attacker, Player *opponent, char coord[]) {
    if ((*attacker).radarUses <= 0) {
        printf("no radar left, your turn is skipped\n");
        return;
    }
    
    // Convert the coordinate 
    int row = coord[1] - '1';         
    int col = coord[0] - 'A';        
    
    // Check if the 2x2 area is in the grid
    if (row < 0 || row > GRID_SIZE - 2 || col < 0 || col > GRID_SIZE - 2) {
        printf("Invalid coordinates, Try again.\n");
        return;
    }
    
    // Check the area for ship
    bool shipsfound = false;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {

            if ((*opponent).smoked[row + i][col + j]) {
                continue;
            }
            char currentCell = (*opponent).own.hide[row + i][col + j];
            if (currentCell == 'C' || currentCell == 'B' || currentCell == 'D' || currentCell == 'S') {
                shipsfound = true;
                break;   // If ship found, we stop 
            }
        }
        if (shipsfound) break;
    }
    
    // result
    if (shipsfound) {
        printf("Enemy ships found\n");
    } else {
        printf("No enemy ships found\n");
    }
    
    // use on counter
    (*attacker).radarUses -= 1;
}
void place_smoke_screen(Player *player, char coord[]) {
   
    int row = coord[1] - '1';         
    int col = coord[0] - 'A';

    
    if (row < 0 || row > GRID_SIZE - 2 || col < 0 || col > GRID_SIZE - 2) {
        printf("Invalid coordinates, Try again.\n");
        return;
    }
    
    // Check if smoke screens left
    if ((*player).smokeUses <= 0) {
        printf("no smoke screens left.turn skipped.\n");
        return;
    }
    
    // Make the 2by 2 grid true
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            (*player).smoked[row + i][col + j] = true;
        }
    }
    
    // Increment smoke screen counter
    (*player).smokeUses -= 1;
    
   
        system("cls"); 
    
    
    printf("Smoke screen placed successfully!\n");
}
void artillery(char coordinate[], Player *attacker, Player *opponent) {
    int column = letterToNumber(coordinate[0]);
    int row = coordinate[1] - '1';

    // Check if the 2x2 area is within the grid bounds
    if (row < 0 || row >= 9 || column < 0 || column >= 9) { 
        printf("Invalid coordinates for artillery!\n");
        return;
    }

    bool hit = false;

    // Check each cell in the 2x2 area
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            int targetRow = row + i;
            int targetCol = column + j;

            // Check if there's a ship part at this cell
            if ((*opponent).own.hide[targetRow][targetCol] != '~') {
                hit = true;
                (*opponent).own.display[targetRow][targetCol] = '*'; // hit
                (*opponent).own.hide[targetRow][targetCol] = '*';    // Update hidden grid
            } else {
                (*opponent).own.display[targetRow][targetCol] = 'O'; // miss
            }
        }
    }

 
    if (hit) {
        printf("Artillery strike successful! Enemy ship(s) hit in the target area.\n");
    } else {
        printf("Artillery strike missed. No ships in the target area.\n");
    }

    // Show the opponent's grid after the attack
    printf("%s's grid after the artillery strike:\n", (*attacker).name);
    printGrid((*opponent).own.display);
}
void torpedo(char coordinate[], Player *attacker, Player *opponent, char direction) {
    int column = letterToNumber(coordinate[0]);
    int row = coordinate[1] - '1';

  
    if (row < 0 || row >= 10 || column < 0 || column >= 10) {
        printf("Invalid coordinates for torpedo!\n");
        return;
    }

    bool hit = false;

    if (direction == 'H' || direction == 'h') { // horizontal 
        for (int j = 0; j < 10; ++j) {
            if ((*opponent).own.hide[row][j] != '~') {
                hit = true;
                (*opponent).own.display[row][j] = '*'; // hit
                (*opponent).own.hide[row][j] = '*';   
            } else {
                (*opponent).own.display[row][j] = 'O'; // miss
            }
        }
    } else if (direction == 'V' || direction == 'v') { // vertical 
        for (int i = 0; i < 10; ++i) {
            if ((*opponent).own.hide[i][column] != '~') {
                hit = true;
                (*opponent).own.display[i][column] = '*'; // hit
                (*opponent).own.hide[i][column] = '*';   
            } else {
                (*opponent).own.display[i][column] = 'O'; // miss
            }
        }
    } else {
        printf("Invalid direction.\n");
        return;
    }

    if (hit) {
        printf("Torpedo hit.\n");
    } else {
        printf("Torpedo miss.\n");
    }

 
    printf("%s'grid after the torpedo strike:\n", (*attacker).name);
    printGrid((*opponent).own.display);
}

