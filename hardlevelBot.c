#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define GRID_SIZE 10


typedef struct {
    char hide[10][10];
    char display[10][10];
    bool smoked[10][10];// added smoked to the phase1 code
} Grid; 

typedef struct {
    char *name;
    int cells;
    int hitCount;
    bool isSunk;// add it to phase1
} Ship;

typedef struct Player {
    bool smoked[GRID_SIZE][GRID_SIZE];
    char *name;
    Grid own;
    Ship ships[4];
    int radarUses;
    int smokeUses;
    int countSunk;
    bool Torpedo;
    bool Artillery;
    bool isBot; // add to code phase1 to Determines if the player is a bot
    bool isTurn; // Tracks whose turn it is
} Player;

//add grid display code from firts phase here

// initializes a player (human or bot)'s grid
void initializePlayer(Player *player, const char *name, bool isBot) {
    player->name= name;
    player->radarUses = 3;
    player->smokeUses = 0;
    player->countSunk = 0;
    player->Torpedo = false;
    player->Artillery = false;
    player->isBot = isBot;
    player->isTurn = false;



    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            player->own.display[i][j] = '~';
            player->own.hide[i][j] = '~';
            player->own.smoked[i][j] = false;// since countsunk 
        }
    }

//to assign ships
    Ship shipss[] = {{"carrier", 5, 0, false}, {"battleship", 4, 0, false}, {"destroyer", 3, 0, false}, {"submarine", 2, 0, false}};
    for (int i = 0; i < 4; i++) {
        player->ships[i] = shipss[i];
    }
}

// Bot makes a strategic moveby taking turn, fire move is included, and adapted phase1 takeTurn to work for human and bot
void takeTurn(Player *attacker, Player *defender, int difficulty) {
    // Display updated opponent (defender) grid before the turn
    printf("Updated grid for %s:\n", defender->name);
    printGrid(defender->own.display);

    if (attacker->isBot) {
        // Bot logic
        int probabilityGrid[GRID_SIZE][GRID_SIZE] = {0};
        calculateProbability(probabilityGrid, defender);

        // Prioritize special moves
        if (attacker->Artillery) {
            botArtilleryAttack(attacker, defender, probabilityGrid);
            attacker->Artillery = false;
            return;
        } else if (attacker->Torpedo) {
            botTorpedoStrike(attacker, defender);
            attacker->Torpedo = false;
            return;
        } else if (attacker->radarUses > 0) {
            botRadar(attacker, defender);
            attacker->radarUses--;
            return;
        } else if (attacker->smokeUses > 0) {
            botSmoke(attacker);
            attacker->smokeUses--;
            return;
        }

        //  fire move based on probability
        int targetRow, targetCol;
        selectTarget(&targetRow, &targetCol, probabilityGrid);

        if (validateCoordinates(targetRow, targetCol)) {
            applyHit(attacker, defender, targetRow, targetCol);
        }
    } else {
        // Human logic
        int move;
        printf("%s, choose your move Fire(1)/Radar(2)/Smoke(3)/Artillery(4)/Torpedo(5): ", attacker->name);
        scanf("%d", &move);

        if (move == 1) {
            fire(attacker, defender);
        } else if (move == 2) {
            printf("Radar Sweep selected!\n");
            Radar(attacker, defender, false); // Human-specific Radar
        } else if (move == 3) {
            printf("Smoke Screen selected!\n");
            Smoke(attacker, false); // Human-specific Smoke
        } else if (move == 4) {
            printf("Artillery Attack selected!\n");
            artilleryAttack(attacker, defender, false, NULL); // Human-specific Artillery
        } else if (move == 5) {
            printf("Torpedo selected!\n");
            torpedoStrike(attacker, defender, false, NULL); // Human-specific Torpedo
        } else {
            printf("Invalid move!\n");
        }
    }

    // Display updated opponent (defender) grid after the turn
    printf("Updated grid for %s:\n", defender->name);
    printGrid(defender->own.display);

    if (!attacker->isBot) {
        // Human-specific prompt to pass the device
        printf("Your turn is over, press any letter and pass the device to %s\n", defender->name);
        char str[2];
        scanf("%s", &str);
        system("cls");
    }
}



// calculate probability grid
void calculateProbability(int probabilityGrid[GRID_SIZE][GRID_SIZE], Player *human) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (human->own.display[i][j] == '~') { 
                for (int k = 0; k < 4; k++) {
                    Ship *ship = &human->ships[k];
                    if (!ship->isSunk) {
                        // check horizontal placement
                        if (j + ship->cells <= GRID_SIZE) {
                            probabilityGrid[i][j]++;
                        }
                        // vertical placement
                        if (i + ship->cells <= GRID_SIZE) {
                            probabilityGrid[i][j]++;
                        }
                    }
                }
            }
        }
    }
}

// selecting the highestprobability target
void selectTarget(int *targetRow, int *targetCol, int probabilityGrid[GRID_SIZE][GRID_SIZE]) {
    int maxProbability = -1;

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (probabilityGrid[i][j] > maxProbability) {
                maxProbability = probabilityGrid[i][j];
                *targetRow = i;
                *targetCol = j;
            }
        }
    }
}

// Artillery attack, same as phase1, adjusted for both humans, and human witha  bot
void artilleryAttack(Player *attacker, Player *opponent, bool isBot, int probabilityGrid[GRID_SIZE][GRID_SIZE]) {
    if (!attacker->Artillery) {
        printf("Artillery move not unlocked!\n");
        return;
    }
    int sunkshipprev = attacker->countSunk;

    int topLeftCol, topLeftRow;

    if (isBot) {
        // Bot logic: select target based on probability
        selectTarget(&topLeftRow, &topLeftCol, probabilityGrid);
        printf("Bot uses Artillery at %c%d!\n", 'A' + topLeftCol, topLeftRow + 1);
    } else {
        // Human logic: get input from the user
        char coord[4];
        printf("%s, enter coordinates for artillery strike (top-left of 2x2 area): ", attacker->name);
        scanf("%s", coord);

        // convert input to grid coordinates
        topLeftCol = letterToNumber(coord[0]);
        if (coord[1] == '1' && coord[2] == '0') {
            topLeftRow = 9; 
        } else if (coord[1] == '1' && coord[2] != 0) {
            topLeftRow = 20; 
        } else {
            topLeftRow = coord[1] - '1'; 
        }
    }

    //validate if the 2x2 area is within grid bounds
    if (!validateCoordinates(topLeftRow, topLeftCol) ||
        !validateCoordinates(topLeftRow + 1, topLeftCol + 1)) {
        if (isBot) {
            printf("Bot selected an invalid artillery target area. Turn skipped.\n");
        } else {
            printf("Invalid artillery target area! You lose your turn.\n");
        }
        return;
    }

    bool hit = false;

    // check each cell in the 2x2 area
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            int row = topLeftRow + i;
            int col = topLeftCol + j;
            char cellInHide = opponent->own.hide[row][col];
            char cellInDisplay = opponent->own.display[row][col];

            if (cellInHide != '~') { // it's a ship
                opponent->own.display[row][col] = '*';
                hit = true; // mark that a hit occurred
                for (int k = 0; k < 4; k++) {
                    if (cellInHide == opponent->ships[k].name[0]) { 
                        opponent->ships[k].hitCount++; 

                        if (isShipSunk(&opponent->ships[k])) { 
                            printf("You sunk a %s!\n", opponent->ships[k].name);
                            attacker->countSunk++;
                            attacker->Artillery = true;  
                            attacker->smokeUses++;      
                        }

                        if (attacker->countSunk == 3) {
                            attacker->Torpedo = true; 
                        }
                        break; 
                    }
                }
            } else { 
                if (difficulty == 1) { 
                    opponent->own.display[row][col] = 'o'; 
                }
            }
        }
    }

    if (hit) {
        printf("Hit!\n");
    } else {
        printf("Miss!\n");
    }

    if (sunkshipprev == attacker->countSunk) {
        attacker->Artillery = false;
        attacker->Torpedo = false;
    }

    
    printf("%s's grid after the artillery strike:\n", opponent->name);
}


// TorpedoStrike, I adjusted the phase1 code again to adapt to when we have a bot playing
void torpedoStrike(Player *attacker, Player *opponent, bool isBot, int probabilityGrid[GRID_SIZE][GRID_SIZE]) {
    if (!attacker->Torpedo) {
        printf("Torpedo move not unlocked!\n");
        return;
    }

    int line = -1;  // row or column index
    bool isRow = true;  // true for row attack, false for column attack

    if (isBot) {
        // Bot logic:with probability to decide row or column
        int rowProb[GRID_SIZE] = {0}, colProb[GRID_SIZE] = {0};

        // calculate probabilities for rows and columns
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (opponent->own.display[i][j] == '~') {  
                    rowProb[i]++;
                    colProb[j]++;
                }
            }
        }
        int maxIndex = 0, maxValue = -1;
        for (int i = 0; i < GRID_SIZE; i++) {
            if (rowProb[i] > maxValue) {
                maxValue = rowProb[i];
                maxIndex = i;
                isRow = true;
            }
            if (colProb[i] > maxValue) {
                maxValue = colProb[i];
                maxIndex = i;
                isRow = false;
            }
        }
        line = maxIndex;  
        printf("Bot uses Torpedo on %s %d!\n", isRow ? "row" : "column", line + 1);
    } else {
        // human logic: prompt for row or column choice
        int choice;
        printf("Row(1) or Column(2)?\n");
        scanf("%d", &choice);

        if (choice == 1) {
            isRow = true;
            printf("Choose a row from 1 to 10: ");
            scanf("%d", &line);
            line--;  
        } else {
            isRow = false;
            char c;
            printf("Choose a column from A to J: ");
            scanf(" %c", &c);
            line = letterToNumber(c);  
        }
    }

    bool hit = false;

    if (isRow) {
        
        for (int j = 0; j < GRID_SIZE; j++) {
            char cell = opponent->own.hide[line][j];
            if (cell != '~' && cell != '*') {  
                hit = true;
                opponent->own.display[line][j] = '*';
                opponent->own.hide[line][j] = '*';

                
                for (int i = 0; i < 4; i++) {
                    if (cell == opponent->ships[i].name[0]) {
                        opponent->ships[i].hitCount++;
                        if (isShipSunk(&opponent->ships[i])) {
                            printf("You sunk a %s!\n", opponent->ships[i].name);
                            attacker->countSunk++;

                           
                            attacker->Artillery = true;
                            attacker->smokeUses++;
                            if (attacker->countSunk == 3) {
                                attacker->Torpedo = true;
                            }
                        }
                        break;
                    }
                }
            } else if (cell == '~') {
                opponent->own.display[line][j] = 'O';  
            }
        }
    } else {
        
        for (int i = 0; i < GRID_SIZE; i++) {
            char cell = opponent->own.hide[i][line];
            if (cell != '~' && cell != '*') {  
                hit = true;
                opponent->own.display[i][line] = '*';
                opponent->own.hide[i][line] = '*';

               
                for (int k = 0; k < 4; k++) {
                    if (cell == opponent->ships[k].name[0]) {
                        opponent->ships[k].hitCount++;
                        if (isShipSunk(&opponent->ships[k])) {
                            printf("You sunk a %s!\n", opponent->ships[k].name);
                            attacker->countSunk++;

                            
                            attacker->Artillery = true;
                            attacker->smokeUses++;
                            if (attacker->countSunk == 3) {
                                attacker->Torpedo = true;
                            }
                        }
                        break;
                    }
                }
            } else if (cell == '~') {
                opponent->own.display[i][line] = 'O';  
            }
        }
    }

    
    if (hit) {
        printf("Torpedo hit!\n");
    } else {
        attacker->Artillery = false;
        printf("Torpedo miss.\n");
    }

    
    attacker->Torpedo = false;
}

// Radar, adapted to having or not having a bot
void Radar(Player *attacker, Player *opponent, bool isBot) {
    
    attacker->Torpedo = false;
    attacker->Artillery = false;

    
    if (attacker->radarUses <= 0) {
        if (isBot) {
            printf("Bot has no radar scans left. Turn skipped.\n");
        } else {
            printf("No radar scans left. Your turn is skipped.\n");
        }
        return;
    }

    int topLeftRow = -1, topLeftCol = -1;

    if (isBot) {
        
        for (int i = 0; i < GRID_SIZE - 1; i++) {
            for (int j = 0; j < GRID_SIZE - 1; j++) {
                if (opponent->own.display[i][j] == '~' &&
                    opponent->own.display[i + 1][j] == '~' &&
                    opponent->own.display[i][j + 1] == '~' &&
                    opponent->own.display[i + 1][j + 1] == '~') {
                    topLeftRow = i;
                    topLeftCol = j;
                    printf("Bot uses Radar at %c%d!\n", 'A' + j, i + 1);
                    break;
                }
            }
            if (topLeftRow != -1) break;
        }
    } else {
  
        char coord[4];
        printf("%s, enter coordinates for radar scan (top-left of 2x2 area): ", attacker->name);
        scanf("%s", coord);

       
        topLeftCol = letterToNumber(coord[0]);
        if (coord[1] == '1' && coord[2] == '0') {
            topLeftRow = 9; 
        } else if (coord[1] == '1' && coord[2] != 0) {
            topLeftRow = 20; 
        } else {
            topLeftRow = coord[1] - '1'; 
        }
    }

    
    if (!validateCoordinates(topLeftRow, topLeftCol) ||
        !validateCoordinates(topLeftRow + 1, topLeftCol + 1)) {
        if (isBot) {
            printf("Bot selected an invalid radar scan area. Turn skipped.\n");
        } else {
            printf("Invalid radar scan area! You lose your turn.\n");
        }
        return;
    }

    
    bool shipsFound = false;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
           
            if (opponent->smoked[topLeftRow + i][topLeftCol + j]) {
                continue;
            }

            char currentCell = opponent->own.hide[topLeftRow + i][topLeftCol + j];
            if (currentCell == 'c' || currentCell == 'b' || currentCell == 'd' || currentCell == 's') {
                shipsFound = true;
                break; 
            }
        }
        if (shipsFound) break;
    }

    
    if (shipsFound) {
        if (isBot) {
            printf("Bot's radar detected enemy ships in the area!\n");
        } else {
            printf("Enemy ships found in the area!\n");
        }
    } else {
        if (isBot) {
            printf("Bot's radar detected no enemy ships in the area.\n");
        } else {
            printf("No enemy ships found in the area.\n");
        }
    }

    
    attacker->radarUses--;
}




void Smoke(Player *attacker, bool isBot) {
    
    attacker->Artillery = false;
    attacker->Torpedo = false;


    if (attacker->smokeUses <= 0) {
        if (isBot) {
            printf("Bot has no smoke screens left. Turn skipped.\n");
        } else {
            printf("No smoke screens left. Turn skipped.\n");
        }
        return;
    }

    int topLeftRow = -1, topLeftCol = -1;

    if (isBot) {
       
        for (int i = 0; i < GRID_SIZE - 1; i++) {
            for (int j = 0; j < GRID_SIZE - 1; j++) {
                if (attacker->own.hide[i][j] != '~') { 
                    topLeftRow = i;
                    topLeftCol = j;
                    printf("Bot places Smoke at %c%d!\n", 'A' + j, i + 1);
                    break; 
                }
            }
            if (topLeftRow != -1) break;
        }
    } else {
        // Human logic: prompt for coordinates
        char coord[4];
        printf("%s, enter coordinates for smoke screen (top-left of 2x2 area): ", attacker->name);
        scanf("%s", coord);

        
        topLeftCol = letterToNumber(coord[0]);
        if (coord[1] == '1' && coord[2] == '0') {
            topLeftRow = 9; 
        } else if (coord[1] == '1' && coord[2] != 0) {
            topLeftRow = 20; 
        } else {
            topLeftRow = coord[1] - '1'; 
        }
    }

    
    if (!validateCoordinates(topLeftRow, topLeftCol) ||
        !validateCoordinates(topLeftRow + 1, topLeftCol + 1)) {
        if (isBot) {
            printf("Bot selected an invalid smoke area. Turn skipped.\n");
        } else {
            printf("Invalid smoke screen area! You lose your turn.\n");
        }
        return;
    }

  
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            attacker->own.smoked[topLeftRow + i][topLeftCol + j] = true;
        }
    }

    
    attacker->smokeUses--;

    
    system("cls");

    printf("Smoke screen placed successfully!\n");
}

//hardlevel fct 
void hardLevel(Player *human, Player *bot) {
    // Initialize players
    initializePlayer(human, "Human", false);
    initializePlayer(bot, "Bot", true);

    // Game loop
    while (true) { 
    //loops until one of them wins or if the game ends
        if (human->isTurn) {
            // Human's turn
            takeTurn(human, bot, 2);  

            // Check if the bot has lost
            if (isGameOver(bot)) {
                displayWinner(human);  // fisplay human as the winner
                break;
            }
        } else {
            // bot's turn
            int probabilityGrid[GRID_SIZE][GRID_SIZE] = {0};
            calculateProbability(probabilityGrid, human);

            if (bot->Artillery) {
                botArtilleryAttack(bot, human, probabilityGrid);
                bot->Artillery = false;
            } else if (bot->Torpedo) {
                botTorpedoStrike(bot, human);
                bot->Torpedo = false;
            } else if (bot->radarUses > 0) {
                botRadar(bot, human);
                bot->radarUses--;
            } else if (bot->smokeUses > 0) {
                botSmoke(bot);
                bot->smokeUses--;
            } else {
                // default Fire
                int targetRow, targetCol;
                selectTarget(&targetRow, &targetCol, probabilityGrid);
                if (validateCoordinates(targetRow, targetCol)) {
                    applyHit(bot, human, targetRow, targetCol);
                }
            }

           
            if (isGameOver(human)) {
                displayWinner(bot);  
                break;
            }
        }

  
        switchTurns(human, bot);
    }
}



// fct to hit somewhere
void applyHit(Player *bot, Player *human, int row, int col) {
    char *cell = &human->own.hide[row][col];
    char *displayCell = &human->own.display[row][col];

    if (*cell != '~' && *cell != '*') {
        *cell = '*';
        *displayCell = '*';
        printf("Bot hits at %c%d!\n", 'A' + col, row + 1);

        for (int i = 0; i < 4; i++) {
            if (human->ships[i].name[0] == *cell) {
                human->ships[i].hitCount++;
                if (human->ships[i].hitCount == human->ships[i].cells) {
                    human->ships[i].isSunk = true;
                    printf("Bot sunk your %s!\n", human->ships[i].name);
                }
                break;
            }
        }
    } else {
        *displayCell = 'O';
        printf("Bot misses at %c%d.\n", 'A' + col, row + 1);
    }
}



// the main function
int main() {
    Player player1, player2;

    printf("Select Difficulty Level: 1-Easy, 2-Medium, 3-Hard: ");
    int difficulty;
    scanf("%d", &difficulty);

    if (difficulty == 1) {
        easyLevel(&player1, &player2); 
    } else if (difficulty == 2) {
        mediumLevel(&player1, &player2);  
    } else if (difficulty == 3) {
        hardLevel(&player1, &player2);  
    } else {
        printf("Invalid difficulty selection!\n");
    }

    return 0;
}


