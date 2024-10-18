#include <stdio.h>
#include <stdlib.h>
typedef struct grid
{
    char display[10][10]; // dsiplay ~ * or 0
    char hide[10][10];    // contains the corresponding letter of each ship in each taken postion and ~ otherwise

} Grid;

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

int letterToNumber(char letter)
{
    return letter - 'A';
}


int check(char position[], char hide[10][10], int cells, char ship) {
    int column = letterToNumber(position[0]); // Convert letter to column index
    int row = position[1] - '1'; // Convert character to zero-indexed row (e.g., '1' -> 0)
    char direction = position[3]; // Read direction from position

    // Check if the initial row and column are within bounds
    if (row < 0 || row >= 10 || column < 0 || column >= 10) {
        return 0; // Invalid position
    }

    // Check for placement based on direction
    if (direction == 'h') {
        // Check if placing horizontally would go out of bounds
        if (column + cells > 10) {
            return 0; // Out of bounds
        }

        // Check if the cells are available
        for (int i = 0; i < cells; i++) {
            if (hide[row][column + i] != '~') { // Assuming '~' means available
                return 0; // Cell is already occupied
            }
        }

        // Place the ship horizontally
        for (int i = 0; i < cells; i++) {
            hide[row][column + i] = ship; // Place the ship
        }
    } else if (direction == 'v') {
        // Check if placing vertically would go out of bounds
        if (row + cells > 10) {
            return 0; // Out of bounds
        }

        // Check if the cells are available
        for (int i = 0; i < cells; i++) {
            if (hide[row + i][column] != '~') { // Assuming '~' means available
                return 0; // Cell is already occupied
            }
        }

        // Place the ship vertically
        for (int i = 0; i < cells; i++) {
            hide[row + i][column] = ship; // Place the ship
        }
    } else {
        return 0; // Invalid direction
    }

    printf("Placement successful.\n");
    return 1; // Success
}


typedef struct player
{
    char *name;
    int isTurn;
    int countSunk;
    Grid own;
} Player;

typedef struct ship
{
    char *name;
    int cells;
} Ship;

int main()
{
    Ship ships[] = {{"carrier", 5}, {"battleship", 4}, {"destroyer", 3}, {"submarine", 2}};
    Player player1;
    Player player2;
    Grid grid1;
    Grid grid2;

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
    enum difficultyLevels
    {
        easy,
        hard
    };
    int difficulty;
    // infrom players about easy and difficult modes
    printf("Please select difficulty level: insert 0 for easy and 1 for hard: ");
    scanf("%d", &difficulty);
    char name1[50];
    char name2[50];

    printf("Player 1 please insert your name: ");
    scanf("%s", &name1);
    player1.name = name1;

    printf("Player 2 please insert your name: ");
    scanf("%s", &name2);
    player2.name = name2;

    int randomPlayer = 1 + (rand() % 2);
    int whosturn = randomPlayer;
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
    printf("Player %d will start filling his grid\n", whosturn);
    char position[4];
    if (player1.isTurn)
    {
        printGrid(grid1.display);
        for (int i = 0; i < 4; i++)
        {
            printf("Please enter the position of the %s: ", ships[i].name);
            scanf("%s", &position);
            // check position
            if (check(position, grid1.hide, ships[i].cells, ships[i].name[0]) == 0)
            {
                printf("Invalid coordinates\n");
                i--;
            }
            printGrid(grid1.hide);
        }
        printf("Now player's 2 turn to fill his grid\n");
        printGrid(grid2.display);
        for (int i = 0; i < 4; i++)
        {
            printf("Please enter the position of the %s: ", ships[i].name);
            scanf("%s", &position);
            if (check(position, grid2.hide, ships[i].cells, ships[i].name[0]) == 0)
            {
                printf("Invalid coordinates\n");
                i--;
            }
            printGrid(grid2.hide);
        }
    }
    else // it is player's 2 turn first
    {
        printGrid(grid2.display);
        for (int i = 0; i < 4; i++)
        {
            printf("Please enter the position of the %s: ", ships[i].name);
            scanf("%s", &position);
            if (check(position, grid2.hide, ships[i].cells, ships[i].name[0]) == 0)
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
            printf("Please enter the position of the %s: ", ships[i].name);
            scanf("%s", &position);
            if (check(position, grid1.hide, ships[i].cells, ships[i].name[0]) == 0)
            {
                printf("Invalid coordinates\n");
                i--;
            }
            printGrid(grid1.hide);
        }
    }

    return 0;
}
