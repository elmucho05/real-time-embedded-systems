#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define CARTA 0
#define SASSO 1
#define FORBICE 2

const char *nomi_mosse[] = {"carta", "sasso", "forbice"};
/*
 * i can try and do the following:
 * - a cond variable for threads player1 and player2
 * - a cond variable for the referee
 * - a mutex
 * - counters for active
 * - counters for blocked
*/
#define NONE -1
#define ROCK 0
#define PAPER 1
#define SCISSORS 2


pthread_mutex_t mutex;
pthread_cond_t cv_players_ready;//referee waits for players to be ready
pthread_cond_t g_waiting_players;//waiting for players
pthread_cond_t g_game_start;//players wait for refere
pthread_cond_t g_players_done; //where the referee waits for players to make the moove
pthread_cond_t g_game_over;// wait for game to end

int player1_ready = false;
int player2_ready = false;

int player1_move = NONE;
int player2_move = NONE;

int game_started = false;
int game_finished = false;


void *player(void *arg){
    int playerId = *(int *)arg;

    while(true){
        //say a player starts
        //1. what do i do?
        if(playerId == 1)
            player1_ready = true;
        else if(playerId == 2)
            player2_ready =true;
        
        printf("player[%d] is now ready\n", playerId);
        //i also have to signal to someone waiting for me 
        pthread_cond_signal(&cv_players_ready);
        
        //2. when do i have to block myself?
        while(!game_started){
            printf("player[%d] blocking \n",playerId);
            pthread_cond_wait(&g_game_start,&mutex);//wait for game to start
        }

        //3. I received start the game
        //i have to pick a move

        int random = rand()%3;
        if(playerId == 1){
            player1_move = random;
            printf("player_1 has chosen [%s]\n", nomi_mosse[player1_move]);

        }else if(playerId == 2){
            player2_move = random;
            printf("player_2 has chosen [%s]\n", nomi_mosse[player2_move]);   
        }

        //4. Now i have to say that i chose
        pthread_cond_signal(&g_players_done);

        //5. block waiting for end_game signal
        while(!game_finished){
            printf("player[%d] is now done and waiting for gameover\n",playerId);
            pthread_cond_wait(&g_game_over, &mutex);
        }

        printf("player[%d] got terminated game_over!\n",playerId);       
        
    }
}


void *referee(void *arg){
    while(true){
        //refereee starts the game
        
        printf("[REFEREE] Hello... Are the players ready? ...\n");
        while(!(player1_ready && player2_ready))
            pthread_cond_wait(&cv_players_ready, &mutex);
        
        //2. start the game
        printf("[REFEREE] Players are all ready, starting the game ...\n");
        game_started = true;
        pthread_cond_broadcast(&g_game_start);

        sleep(2); // Simulate delay

        //3. wait for players to make the move
        //check if they have made a move 
        while(player1_move==NONE || player2_move==NONE){
            printf("[REFEREE] waiting for players to make their move ...\n");
            pthread_cond_wait(&g_players_done, &mutex);
        }

        //4. Analize the moves
        printf("[REFEREE] All players are done! %s vs %s \n",nomi_mosse[player1_move], nomi_mosse[player2_move]);
        printf("[REFEREE] Terminate the game.\n");
        printf("\n=========================\n");
        
        // 5. Reset game state
        player1_ready = false;
        player2_ready = false;
        player1_move = NONE;
        player2_move = NONE;

        game_started = false;
        
        game_finished = true;

        pthread_cond_broadcast(&g_game_over);
        pthread_cond_wait(&g_waiting_players,&mutex);

        sleep(2);

    }
}
int main(){
	pthread_t p1,p2,ref;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cv_players_ready,NULL);
    pthread_cond_init(&g_waiting_players,NULL);
    pthread_cond_init(&g_players_done,NULL);
    pthread_cond_init(&g_game_over,NULL);
    pthread_cond_init(&g_game_start,NULL);

	/* inizializzo il sistema */
	/* inizializzo i numeri casuali, usati nella funzione pausetta */
	srand(time(NULL));//sempre gli stessi numeri, ad ogni esecuzione del programma
	int player1_id = 1;
	int player2_id = 2;
	pthread_create(&p1, NULL, player, &player1_id);
	pthread_create(&p2, NULL, player, &player2_id);
	pthread_create(&ref, NULL, referee, NULL);
	pthread_join(p1, NULL);
	pthread_join(p2, NULL);
	pthread_join(ref, NULL);



	return 0;
}