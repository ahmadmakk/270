#include <stdio.h>
#include <stdlib.h>

/* we define the struct grid
 each player will later have their own grid structure
 in it, one to be attacked by the oppoenet player (displayed at opponents turn)
 and another to keep record of where own's ships are (hidden to opponent )
 the one to be displayed shows the water(~) the hits(*) and the misses(0)
 the hidden one shows the letter of the ship where it is placed and water(~) otherwise
*/
typedef struct grid
{
    char display[10][10]; 
    char hide[10][10];   
} Grid;


/* we define a player structure 
to keep record of the player's name
turn to play(0 if no 1 if yes)
number of oponent's ships they successfully sank 
and their own grid
*/
typedef struct player
{
    char *name;
    int isTurn;
    int countSunk;
    // we can change the naming to make it meaningful
    Grid own;
    Ship ships[4]; //Nours' addition

} Player;

/* we define a ship structure
to keep record of each's ship name with the it corresponding size (cells) and hitcount (times it has been hit)
*/
typedef struct ship
{
    char *name;
    int cells;
    int hitCount; // I added this because we need it to check later if the ship has been sunk or no
} Ship;



//a function to print a 2D array with letters in the first row and indices in the first colomn

/*
since it might look complicated as it links the letters and the indices to i and j
 I suggest this more readable format of it 

void printGrid(char arr[10][10]){

    printf("    A  B  C  D  E  F  G  H  I  J\n");

    for (int i = 0; i < 10; i++) {
     printf("%2d ", i + 1);  
      for (int j = 0; j < 10; j++ ){
         printf("%c ", arr[i][j]);
         }
         printf(\n);
    }

*/
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

// a function that takes a letter as an argument and returns a number representing its position in the alphabets
// so letterToNumber(A) returns 0 and letterToNumber(B) returns 1 ... 
// this function will later help us in manipulating the columns as they are represented by letters in our grids
int letterToNumber(char letter)
{
    return letter - 'A';
}


/*check function takes as an argument:
-the position of the ship to be placed as an array of [letter of colomn (x coordinate), number of row (y coordinate), direction (vertical or horizontal)]
-the 2D array hide of the player that keeps record of the placement of ships deployed
-the number of cells occupied by the ship (masalan destroyer is 3)
-the initial of the ship (masalan destroyer is "d")

all of these are neccessary to check whether a ship can be deployed by the player in the position specified
if the position is alright we return 1 otherwise 0
*/

// we can add print statements to inform the player of failed positions
int check(char position[], char hide[10][10], int cells, char ship) {

    //  Convert letter of colomn to index
    int column = letterToNumber(position[0]); 

    // here we just decremt 1 from the row to make it aligned with 0 indexing 
    // we should account for row 10 taking two slots at 1 and 2
    int row = position[1] - '1'; 
    
    // store the direction in a char
    char direction = position[3]; 

    // Check if row and column are within bounds so between 0 and 10
    if(!validateCoordinates(row,column)) return 0; //unvalid position


    // Check for placement based on direction

    // if direction is horizontal
    if (direction == 'h') {

        // Check if placing horizontally would go out of bounds
        if (column + cells > 10) {
            return 0;// unvalid position
        }

        // Check if the cells are available
        for (int i = 0; i < cells; i++) {
            if (hide[row][column + i] != '~') { // since '~' means available
                return 0; // Cell is occupied so unvalid position
            }
        }

        // position is valid so we place it
        for (int i = 0; i < cells; i++) {
            hide[row][column + i] = ship; // Place the ship by adding its initial 
        }
    } 

    // if direction is vertical we do similar to horizontal on row level
    else if (direction == 'v') {
        if (row + cells > 10) {
            return 0;
        }
        for (int i = 0; i < cells; i++) {
            if (hide[row + i][column] != '~') {
                return 0; 
            }
        }
        for (int i = 0; i < cells; i++) {
            hide[row + i][column] = ship; 
        }
    } 
    // direction is not vertical or horizontal!unvalid
    else {
        return 0; 
    }


    printf("Placement successful.\n");
    return 1; // Success
}



// Nour's code in the preprocessor

// the following is a function to switch the turn between the two players
void switchTurns(Player *p1, Player *p2)
{
    if (p1->isTurn)//p1 turn is 1
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

   //we examine all the movements possibilities
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
    else if (!strcmp(move, "Smoke") )
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

    int column =letterToNumber(coord[0]);

    int row;
     if (coord[1] == '1' && coord[2] == '0') { // Checking for "10"
    row = 9; // Since the grid is zero-indexed, row 10 becomes 9
      } else {
    row = coord[1] - '1'; // For single-digit rows
      }

     
      
    if (validateCoordinates(row, column))
    {
       char cellinHide = defender->own.hide[row][column];
       char cellinDisplay = defender->own.display[row][column];

       if (cellinDisplay=="*"){
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
                     attacker-> countSunk++;
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



    player1.own  = grid1;
    player2.own  = grid2;


    //Nour's addition
     for (int i = 0; i < 4; i++) {
    player1.ships[i] = shipss[i]; 
    player2.ships[i] = shipss[i];
      }

    //initializing each player's grid (both the displayed and the hidden) to have all water(~)
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

    //defining difficulty levels
    enum difficultyLevels
    {
        easy,//0
        hard//1
    };
    int difficulty;


    // asking players to select easy or difficult modes
    printf("Please select difficulty level: insert 0 for easy and 1 for hard: ");
    scanf("%d", &difficulty);

    //declaring two char arrays to store the name of the players
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

    // randomly choosing who will start first
    // refresh time to produce new random numbers
    int randomPlayer = 1 + (rand() % 2);
    int whosturn = randomPlayer;

    //informing the players who will start first
    printf("Player %d will start filling his grid\n", whosturn);


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
  


    // declaring position array for ships
    char position[5];

    //if it was the turn of player one
    if (player1.isTurn)
    {
      // first we print their grid (which's initially all water)
        printGrid(grid1.display);

       // a for loop to pass over all the ships in the ships array
        for (int i = 0; i < 4; i++)
        {
            // asking the player for the position and storing it
            printf("Please enter the position of the %s: ", shipss[i].name);
            scanf("%s", &position);
            
            // checking if the position is valid with our check function 
            if (!check(position, grid1.hide, shipss[i].cells, shipss[i].name[0]))
            {
                printf("Invalid coordinates\n");// should be informed in the function check
                i--; //decremnting i so we pass over the ship again and the player get to deploy it correclty
            }

            /*after each placement the player is shown their grid
            */
            printGrid(grid1.hide);
        }
       

        // now the turn of player 2 to deploy their ships
        printf("Now player's 2 turn to fill their grid\n");
        printGrid(grid2.display);
        for (int i = 0; i < 4; i++)
        {
            printf("Please enter the position of the %s: ", shipss[i].name);
            scanf("%s", &position);
            if (check(position, grid2.hide, shipss[i].cells, shipss[i].name[0]) == 0)
            {
                printf("Invalid coordinates\n");
                i--;
            }
            printGrid(grid2.hide);
        }
    }

  // same thing but if player two was the one who played first
    else 
    {
        printGrid(grid2.display);
        for (int i = 0; i < 4; i++)
        {
            printf("Please enter the position of the %s: ", shipss[i].name);
            scanf("%s", &position);
            if (check(position, grid2.hide, shipss[i].cells, shipss[i].name[0]) == 0)
            {
                printf("Invalid coordinates\n");
                i--;
            }
            printGrid(grid2.hide);
        }
        printf("Now player's 1 turn to fill his grid\n");
        printGrid(grid1.display);
        for (int i = 0; i < 4; i++)
        {
            printf("Please enter the position of the %s: ", shipss[i].name);
            scanf("%s", &position);
            if (check(position, grid1.hide, shipss[i].cells, shipss[i].name[0]) == 0)
            {
                printf("Invalid coordinates\n");
                i--;
            }
            printGrid(grid1.hide);
        }
    }
       /*calling them player1 and player2 is somehow misleading as player2 might have turn 1
        so let's call them by their names stored in their structs it's clearer*/
       // seems like a lot of repeated code we can avoid that?



// Nour's code

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
  
    return 0;
}

