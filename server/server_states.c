/*
 * Copyright 2021 D'Arcy Smith + the BCIT CST Datacommunications Option students.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <dcfsm/fsm.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#define PORT 49157
#define BACKLOG 5
#define BUF_SIZE 3

#define PLAYER_X 0
#define PLAYER_O 1

#define VALID 1
#define INVALID 0

#define WIN 1
#define TIE 2
#define GAME_CONTINUES 0

#define STARTING_BOARD {'-', '-', '-', '-', '-', '-', '-', '-', '-'}

char PLAYER_SYMBOLS[2] = {'X', 'O'};

char WINNING_MOVES[8][3] = { 
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, 
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
    {0, 4, 8}, {2, 4, 6} };




static int forming_team(Environment *env);
static int starting_game(Environment *env);
static int awaiting_player(Environment *env);
static int evaluating_move(Environment *env);
static int ending_game(Environment *env);
static int broken_game(Environment *env);
static int get_server_socket();


typedef enum
{
    FORMING_TEAM = FSM_APP_STATE_START,
    STARTING_GAME,
    AWAITING_PLAYER,
    EVALUATING_MOVE,
    ENDING_GAME,
    BROKEN_GAME
} States;


typedef struct
{
    Environment common;
    int server_fd;
    int move_count;
    int winning_player;
    int player_fds[2];
    int current_player;
    char received_move[3];
    char game_board[9];
} GameEnvironment;

static void print_board(char* board){
    for(int i = 0; i < 9; i++){
        if(i == 3 | i == 6 ) printf("\n");
        printf("%c", board[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    int server_fd;
    server_fd = get_server_socket();
    
    GameEnvironment env;
    env.server_fd = server_fd;
    
    StateTransition transitions[] =
    {
            { FSM_INIT, FORMING_TEAM, &forming_team },
            { FORMING_TEAM, STARTING_GAME, &starting_game },
            { STARTING_GAME, AWAITING_PLAYER, &awaiting_player },
            { AWAITING_PLAYER, EVALUATING_MOVE, &evaluating_move },
            { EVALUATING_MOVE, AWAITING_PLAYER, &awaiting_player },
            { EVALUATING_MOVE, ENDING_GAME, &ending_game },
            { ENDING_GAME, FORMING_TEAM, &forming_team },
            { STARTING_GAME, BROKEN_GAME, &broken_game },
            { BROKEN_GAME, FORMING_TEAM, &forming_team },
            { FORMING_TEAM, FSM_EXIT, NULL },
            { FSM_IGNORE, FSM_IGNORE, NULL },
    };
    int code;
    int start_state;
    int end_state;
    env.move_count = 0;
    start_state = FSM_INIT;
    end_state   = FORMING_TEAM;
    code = fsm_run((Environment *)&env, &start_state, &end_state, transitions);

    if(code != 0)
    {
        fprintf(stderr, "Cannot move from %d to %d\n", start_state, end_state);

        return EXIT_FAILURE;
    }

//    fprintf(stderr, "Exiting state %d\n", start_state);

    return EXIT_SUCCESS;
}

static int forming_team(Environment *env)
{
    printf("ENTERED STATE: forming_team\n");

    GameEnvironment *game_env;
    game_env = (GameEnvironment *) env;
    game_env->current_player = PLAYER_X;

    int server_fd = game_env->server_fd;

    int client_1 = accept(server_fd, NULL, NULL);
    printf("connected client 1\n");
    int client_2 = accept(server_fd, NULL, NULL);
    printf("connected client 2\n");

    // Randomizing starting player
    if (rand() % 2 == 0) {
        game_env->player_fds[0] = client_1;
        game_env->player_fds[1] = client_2;
    } else {
        game_env->player_fds[0] = client_2;
        game_env->player_fds[1] = client_1;
    }

    return STARTING_GAME;
}

static int starting_game(Environment *env)
{
    printf("ENTERED STATE: starting_game\n");

    GameEnvironment *game_env;
    game_env = (GameEnvironment *) env;

    // Initialize empty board
    for (int i = 0; i < 9; i++) {
        game_env->game_board[i] = '-';
    }

    int player_x = game_env->player_fds[PLAYER_X];
    int player_o = game_env->player_fds[PLAYER_O];
    
    write(player_x, "NX", 2);
    write(player_o, "NO", 2);

    ssize_t num_read;
    char buf[BUF_SIZE];

    num_read = read(player_x, buf, 2);
    if(strcmp(buf, "OK") == 0){
        printf("response from player x: %s\n", buf);
    } else {
        return BROKEN_GAME;
    }
    

    num_read = read(player_o, buf, 2);
    if(strcmp(buf, "OK") == 0){
        printf("response from player o: %s\n", buf);
    } else {
        return BROKEN_GAME;
    }
    

    return AWAITING_PLAYER;
}

static int awaiting_player(Environment *env)
{
    GameEnvironment *game_env;
    game_env = (GameEnvironment *) env;

    ssize_t num_read;
    char buf[BUF_SIZE];
    memset(buf, 0, 3);

    printf("ENTERED STATE: awaiting_player\n");
    int current_player  = game_env->current_player;
    int current_player_fd = game_env->player_fds[current_player];

    printf("current player: %d, current_fd: %d\n", current_player, current_player_fd);

    num_read = read(current_player_fd, buf, 2);
    printf("we got: %s from %c\n",buf, current_player );
    memset(game_env->received_move, 0, 3);
    strncpy(game_env->received_move, buf, 3);

    return EVALUATING_MOVE;
}



static int is_valid_move(Environment *env){

    GameEnvironment *game_env;
    game_env = (GameEnvironment *) env;

    char * move = game_env->received_move;
    char * board = game_env->game_board;

    char moveOp = move[0];
    char moveCode = move[1];

    if (moveOp != 'A') return INVALID;

    if (moveCode > 56 || moveCode < 48) return INVALID;

    if (board[moveCode - 48] != '-')return INVALID;

    board[moveCode - 48] = PLAYER_SYMBOLS[game_env->current_player];

    return VALID;
}

static int is_final_move(GameEnvironment *env) {
    char * board = env->game_board;
    int player = env->current_player;
    int move_count = env->move_count;
    char player_char = PLAYER_SYMBOLS[player];
    //check game win
    int moves_in_a_row = 0;
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 3; j++){
            if(board[WINNING_MOVES[i][j]] == player_char){
                moves_in_a_row++;
            }
        }
        if(moves_in_a_row == 3){
            return WIN;
        }
        moves_in_a_row = 0;
    }

    //check move count 9
    if(move_count == 9){
        return TIE;
    }
    return GAME_CONTINUES;
}

static int evaluating_move(Environment *env)
{
    printf("ENTERED STATE: evaluating_move\n");

    GameEnvironment *game_env;
    game_env = (GameEnvironment *) env;

    char * move = game_env->received_move;
    printf("evaluating: %s\n", move);

    int current_player  = game_env->current_player;
    int other_player = (current_player + 1) % 2;

    int current_player_fd = game_env->player_fds[current_player];
    int other_player_fd = game_env->player_fds[other_player];

    if(!is_valid_move(env)){
        char error_msg[2];
        error_msg[0] = 'I';
        error_msg[1] = move[1];

        write(current_player_fd, error_msg, 2);

        return AWAITING_PLAYER;
    }
    
    (game_env->move_count)++;
    printf("Adding move #%d\n", game_env->move_count);
    print_board(game_env->game_board);

    int move_state = is_final_move(game_env);

    if (move_state == WIN) {
        game_env->winning_player = current_player;
        return ENDING_GAME;
    } else if( move_state == TIE){
        game_env->winning_player = TIE;
        return ENDING_GAME;
    }
    // CHANGING TURNS
    game_env->current_player = other_player;
        
    char success_msg[2];
    success_msg[0] = 'S';
    success_msg[1] = move[1];

    write(current_player_fd, success_msg, 2);
    write(other_player_fd, success_msg, 2);
        
    return AWAITING_PLAYER;
}

static int ending_game(Environment *env)
{
    printf("ENTERED STATE: ending_game\n");

    sleep(2);

    return FORMING_TEAM;
}

static int get_server_socket() {
    struct sockaddr_in addr;
    int sfd;
    
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    
    listen(sfd, BACKLOG);

    return sfd;
}

static int broken_game(Environment *env){
    printf("HOLY FUCK SOMETHING FUCKED UP\n");
    return FORMING_TEAM;
}