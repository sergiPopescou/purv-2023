#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define DIMENSION 10 /* Conway game of life field dimension. */
#define CALC_DIMENSION 3 /* Dimension for matrix which holds cell and neighbourg information. */
#define NUM_OF_PRIORITIES 28 /* Number of possible priorities */

int CELLS_MATRIX[DIMENSION][DIMENSION]; /* Conway game of life main matrix. */
int PRIORITY_IDX = 0; /* Index to next priority value to decide next cell(s) (thread(s)) for evolution. */
int THREAD_COUNTER = 0; /* Internal counter in cell threads to control posting */
int CELLS_TO_EVOL = 0; /* Number of cells for evolution in current post */

static sem_t displaySem; /* Semaphore for main thread, which controls displaying of matrix and starts evolution. */
static sem_t controlSem; /* Semaphore to control controller thread. */
static sem_t cellsSem[NUM_OF_PRIORITIES]; /* Semaphores for cell threads control*/
static pthread_mutex_t thread_counter_mtx = PTHREAD_MUTEX_INITIALIZER; /* Mutex to control access to THREAD_COUNTER variable */

/* Function to calculate next state of cell from it's current state and state of it's neighbourgs. */
int calculate_conway(int cells[][CALC_DIMENSION]){
     int alive_cells = 0;
     int cell_state = cells[CALC_DIMENSION/2][CALC_DIMENSION/2];
     for(int i=0;i<CALC_DIMENSION;i++){
          for(int j=0;j<CALC_DIMENSION;j++){
               alive_cells += cells[i][j];
          }
     }
     alive_cells -= cell_state;
     if((alive_cells == 3) || (alive_cells == 2 && cell_state))
          cell_state = 1;
     else
          cell_state = 0;
     return cell_state;
}

/* Function which displays cells matrix. */
void print_matrix(int mat[][DIMENSION]){
     for(int i=0;i<DIMENSION;i++){
          for(int j=0;j<DIMENSION;j++){
               printf("%c " , mat[i][j] == 1 ? 'o' : '.');
               if(j == DIMENSION - 1)
                    printf("\n");
          }
     }
}

/* Function which initializes cells matrix with random states. */
void init_matrix(int mat[][DIMENSION]){
     srand(time(NULL));
     for(int i=0;i<DIMENSION;i++)
          for(int j=0;j<DIMENSION;j++)
               mat[i][j] = (rand() % 2 == 0) ? 1 : 0;
}

/* Checks validity of an index sent from threads to create matrix of neighbourg cells. */
inline int valid_index(int idx){
     return ((idx >= 0) && (idx < DIMENSION));
}

/* Thread function, each thread is responsible for one cell. In every cycle it creates matrix for calculate_conway 
   and inserts new state in cells matrix for it's responsible cell. */
void* cell_thread_fun(void* param){
     int* pair = (int*) param;
     int cells_scope[CALC_DIMENSION][CALC_DIMENSION];
     while(1){
          sem_wait(&cellsSem[pair[2]]);
          int idx_i = pair[0] - 1;
          int idx_j = pair[1] - 1;
          for(int i=0;i<CALC_DIMENSION;i++){
               for(int j=0;j<CALC_DIMENSION;j++){
                    if(valid_index(idx_i) && valid_index(idx_j)){
                        cells_scope[i][j] = CELLS_MATRIX[idx_i][idx_j];
                    }
                    else{
                         cells_scope[i][j] = 0; 
                    }
                    idx_j++;
               }
               idx_i++;
               idx_j -= CALC_DIMENSION;
          }
          CELLS_MATRIX[pair[0]][pair[1]] = calculate_conway(cells_scope);

          pthread_mutex_lock(&thread_counter_mtx);
               THREAD_COUNTER++;
               if(THREAD_COUNTER == CELLS_TO_EVOL){
                    THREAD_COUNTER = 0;
                    sem_post(&controlSem);
               }
          pthread_mutex_unlock(&thread_counter_mtx);
     }     
}

/* Thread function of controller thread - controls paralelization of conway cells and display of the cells matrix */
void* controller_thread_fun(void* param){
     int post_idx = 0;
     int i = 0;
     while(1){
          sem_wait(&controlSem);

               if(PRIORITY_IDX == 0 || PRIORITY_IDX == 1 || PRIORITY_IDX == 26 || PRIORITY_IDX == 27){
                    CELLS_TO_EVOL = 1;
               }
               else if(PRIORITY_IDX == 2 || PRIORITY_IDX == 3 || PRIORITY_IDX == 24 || PRIORITY_IDX == 25){
                    CELLS_TO_EVOL = 2;
               }
               else if(PRIORITY_IDX == 4 || PRIORITY_IDX == 5 || PRIORITY_IDX == 22 || PRIORITY_IDX == 23){
                    CELLS_TO_EVOL = 3;
               }
               else if(PRIORITY_IDX == 6 || PRIORITY_IDX == 7 || PRIORITY_IDX == 20 || PRIORITY_IDX == 21){
                    CELLS_TO_EVOL = 4;
               }
               else{
                    CELLS_TO_EVOL = 5; 
               }
               post_idx = PRIORITY_IDX;
               if(PRIORITY_IDX == NUM_OF_PRIORITIES - 1){
                    PRIORITY_IDX = 0;
                    sem_post(&displaySem);
                    usleep(50);
               }
               else{
                    PRIORITY_IDX++;
               }
               for(i=0;i<CELLS_TO_EVOL;i++){
                    sem_post(&cellsSem[post_idx]);
               }
     }
}

/* Function to calculate matrix of priorities */
void calculate_priorities(int priorities[][DIMENSION]){
     for(int i=0;i<DIMENSION;i++){
        int offset = i * 2;
        for(int j=0;j<DIMENSION;j++){
            priorities[i][j] = offset++;  
        }
    }
}

/* Main function, cmd line arguments check and proccessing, semaphore and thread initialization and 
     displaying and evolution control. */
int main(int argc, char** argv)
{
     int pairs[DIMENSION*DIMENSION][3];
     int priorities[DIMENSION][DIMENSION];
     pthread_t cell_threads[DIMENSION][DIMENSION];
     pthread_t controller_thread;
     int first = 1;
     if(argc != 2){
          printf("Missing command line args !\n");
          return -1;
     }
     int sleep_time = atoi(argv[1]);
     sem_init(&displaySem,0,1);
     sem_init(&controlSem,0,0);
     for(int i=0;i<NUM_OF_PRIORITIES;i++)
          sem_init(&cellsSem[i],0,0);
     calculate_priorities(priorities);
     int k=0;
     for(int i=0;i<DIMENSION;i++){
          for(int j=0;j<DIMENSION;j++){
               pairs[k][0] = i;
               pairs[k][1] = j;
               pairs[k][2] = priorities[i][j];
               pthread_create(&cell_threads[i][j], NULL, cell_thread_fun, (void*)pairs[k]);
               k++;
          }
     }
     init_matrix(CELLS_MATRIX);
     pthread_create(&controller_thread, NULL, controller_thread_fun, NULL);
     while(1){ 
          sem_wait(&displaySem);

          system("clear");
          print_matrix(CELLS_MATRIX);
          fflush(stdout);
          if(first){
               sem_post(&controlSem);
               first = 0;
          }
          sleep(sleep_time);
     }
     return 0;
}
