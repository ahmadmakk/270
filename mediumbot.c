    // in the main 

    // we should increment the consecutive misses of the bot or zero it according to hits

    // we should check the return of fire and radar and artillery to know if we must update the state of the bot

    //we should make radar and fire and artillery return true or false so we can make use of the return value

    // we should manage where to take the move and the coordinates between the main and the take turns function 

    /* we should manage the sinkings of the ships of the bot in the main
    
    and update artillery to true once there is a sinking so it correctly use it in time  */
    
    // we should deal with the format of the string produced by generate move of bot
    
    //once a ship is sunk we should set the orientation to 0



    #include <stdbool.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <ctype.h>
    #define GRID_SIZE 10

    //already in the main file
    typedef struct grid
    {
        char display[GRID_SIZE][GRID_SIZE];
        char hide[GRID_SIZE][GRID_SIZE];
    } Grid;

    typedef struct ship
    {
        char *name;
        int cells;
        int hitCount; 
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

    int validateCoordinates(int row, int column)
    {
        return (row >= 0 && row < 10 && column >= 0 && column < 10);
    }







    typedef struct {

        Player* base;    

        int haunting;                // if 1 then we need to haunt for targets if 0 then we have targets
        int last_move;               // if 1, the last hit was fire, 2 for radar, 3 for artillery
        char hit_stack[100][3];       // Stack of coordinates to target next
        int hit_stack_size;           // Size of the hit stack
        int ConsecutiveMisses;        /*tracks consecutive misses to see if the bot is struggling
                                        should be 0-ed in the main if the bot hits */
        int orientation;  //if 1 then vertical, if 2 then horizontal, if 0 then we're not even hitting                  

    } MediumBot;




    int letterToNumber(char letter)
    {
        return letter - 'A';
    }



    // the following function genertes the move of a medium bot
    // it returns an array that specifies the move (fire or radar) with the coordinates (the column letter and the row number)


    char* generate_medium_bot_move(MediumBot* MediumBot, Grid PalyerGrid) {

        static char move[20]; // Move string to return

        if(MediumBot->hit_stack_size==0) MediumBot->haunting=1;
        // if the medium bot is not haunting

         if (MediumBot->base->Artillery){
            char artillery_target[3];
            find_best_2by2_area(PalyerGrid, artillery_target);
            sprintf(move, "Artillery %s", artillery_target); 
            MediumBot->last_move = 3;
            return move;
        }

        else if (!MediumBot->haunting) {
            // we set the move to be fire
            strcpy(move, "Fire "); 
            // we concatentate to the move the coordinates found last in the hit stack
            // we decrement the hit stack size
            strcat(move, MediumBot->hit_stack[--(MediumBot->hit_stack_size)]);
            MediumBot->last_move = 1;
            return move;
        }                                                                                               

        // else if we are haunting for places to target
        // and we can still use radar and it is a good choice to make use of it in this turn
        else if (MediumBot->base->radarUses > 0 && should_use_radar(MediumBot, PalyerGrid)) {

            char radar_target[3];
            //finds the best coordiates to tagret by radar and stores it in radar target
            find_best_2by2_area(PalyerGrid, radar_target);
            // we decrement the number of radar sweeps we can still use
            //  move  = Radar + radar target
            sprintf(move, "Radar %s", radar_target); 
            MediumBot->last_move = 2;
            return move;
        }
        // if we're haunting but it's not a good choice to use radar
        else {
            //we'll have to make a guess but not a completely random one
            char guess[3];
            medium_guess(PalyerGrid, guess);
            sprintf(move, "Fire %s", guess);
            MediumBot->last_move = 1;
            return move;
        }
    }

    bool should_use_radar(MediumBot* MediumBot, Grid PlayerGrid) {

    //count how many unkown cells (~) there are
        int unknown_cells = 0;
        for (int row = 0; row < 10; row++) {
            for (int col = 0; col < 10; col++) {
                if (PlayerGrid.display[row][col] == '~') {
                    unknown_cells++;
                }
            }
        }
        
        if (unknown_cells < 50 && MediumBot->ConsecutiveMisses > 3) {
            return 1; // Mid-game and struggling to find targets -> use radar
        } else if (unknown_cells < 25) {
            return 1; // Late game, prioritize finding remaining ships
        }

        return 0; // Avoid radar in the early game (if unkown cells are many  >50)
    }

      
    void medium_guess(Grid PlayerGrid, char* guess) {

        for (int row = 0; row < 10; row++) {
            for (int col = 0; col < 10; col++) {
                if ((row + col) % 2 == 0 && PlayerGrid.display[row][col] == '~') {

                    guess[0] = 'A' + col;     // Convert column to letter
                    sprintf(guess + 1, "%d", row + 1); // Convert row to number
                    return;

                }
            }
        }

    }

    // used for radar and artillery
    void find_best_2by2_area(Grid PlayerGrid,  char* target) {

        int max_water = -1;
        char best_area[3] = "";

        for (int row = 0; row < 9; row++) { 
            for (int col = 0; col < 9; col++) { 

                int water = 0;

                for (int r = row; r < row + 2; r++) {
                    for (int c = col; c < col + 2; c++) {

                        if (PlayerGrid.display[r][c] == '~') {
                            water += 1; 
                    }
                }

                if (water > max_water) {
                    max_water = water;
                    best_area[0] = 'A' + col;
                    sprintf(best_area + 1, "%d", row + 1);
                }
                   if(max_water == 4) break; // we can't reach more than 4 per 2x2 grid
            }
                if(max_water == 4) break;
        }

        strcpy(target, best_area);
    }



    }
    // the following function populates the hit stack with the possible adjacent targets of 
    // a hit once there is one using fire (up left, )
    // radar returns "true" meaning Enemy ships found in the area so we store them in the stack
    // currently radar returns void so we should make it return bool for that
    //artilley hits
    void update_bot_state_after_hit(MediumBot* MediumBot, const char* hit_coord, Grid PlayerGrid) {

        MediumBot->haunting = 0;


        int col = letterToNumber(hit_coord[0]); // Extract column 

        int row;
        if (hit_coord[1] == '1' && hit_coord[2] == '0')
        {          
            row = 9; // Since the grid is zero-indexed, row 10 becomes 9
        }
        else
        {
            row = hit_coord[1] - 1; // For single-digit rows
        }


    // if last move was fire we check adjacent cells depending on orientation
    if(MediumBot->last_move = 1) {
        check_around_hit(MediumBot,PlayerGrid,row,col);
    }
    // if last move was radar we check the 2X2 grid of which hit_coord is the top left corner


    else if (MediumBot->last_move = 2){

        int candidates[4][2] = {{row, col}, {row + 1, col}, {row, col + 1}, {row +1, col + 1}};

        for (int i = 0; i < 4; i++) {
        addto_hit_stack(MediumBot, candidates[i], PlayerGrid);
        }

    } 

    // if last move was artilery we check eaxh cell in the 2X2 grid of which hit_coord is the top left corner
    // if each cell was a hit or not

    else if (MediumBot->last_move = 3){

        int candidates[4][2] = {{row, col}, {row + 1, col}, {row, col + 1}, {row +1, col + 1}};

        for (int i = 0; i < 4; i++) {

            int r = candidates[i][0];
            int c = candidates[i][1];

            if(PlayerGrid.display[r][c]=='*'){
            check_around_hit(MediumBot,PlayerGrid,r,c);
            }
        }

    }


    }



    void addto_hit_stack(MediumBot *MediumBot, int candidate[2], Grid PlayerGrid){
            int row = candidate[0];
            int col = candidate[1];
        if (validateCoordinates(row, col) && PlayerGrid.display[row][col]== '~') {
                sprintf(MediumBot->hit_stack[MediumBot->hit_stack_size++], "%c%d", 'A' + col, row + 1);
            }
    }



    void check_around_hit(MediumBot* MediumBot, Grid PlayerGrid, int row, int col){

     int down_Up_left_right[4][2] = {{row - 1, col}, {row + 1, col}, {row, col - 1}, {row, col + 1}};

        if(MediumBot->orientation == 0){
        if (validateCoordinates(row+1, col) && PlayerGrid.display[row+1][col] == '*' )
        MediumBot->orientation = 1;
        else if (validateCoordinates(row-1, col) && PlayerGrid.display[row-1][col] == '*' )
        MediumBot->orientation = 1;
        
        else if (validateCoordinates(row, col+1) && PlayerGrid.display[row][col+1] == '*' )
        MediumBot->orientation = 2;
        else if (validateCoordinates(row, col-1) && PlayerGrid.display[row][col-1] == '*' )
        MediumBot->orientation = 2;
          }

        // no orientation 
        if(MediumBot->orientation == 0){
        addto_hit_stack(MediumBot, down_Up_left_right[0], PlayerGrid);
        addto_hit_stack(MediumBot, down_Up_left_right[1], PlayerGrid);
        addto_hit_stack(MediumBot, down_Up_left_right[2],PlayerGrid);
        addto_hit_stack(MediumBot, down_Up_left_right[3], PlayerGrid);
        }
        //vertical orientation
        if(MediumBot->orientation == 1){
            if (validateCoordinates(row+1, col) && PlayerGrid.display[row+1][col] == '*')
            addto_hit_stack(MediumBot, down_Up_left_right[1],PlayerGrid);
            else
            addto_hit_stack(MediumBot, down_Up_left_right[0],PlayerGrid);
        }
        // horizontal orienatation
        if(MediumBot->orientation == 2){
            if ( validateCoordinates(row, col+1) && PlayerGrid.display[row][col+1] == '*')
            addto_hit_stack(MediumBot, down_Up_left_right[2], PlayerGrid);
            else
            addto_hit_stack(MediumBot, down_Up_left_right[3], PlayerGrid);
            
        }

    }


    int main (){

        return 0;
    }
   
