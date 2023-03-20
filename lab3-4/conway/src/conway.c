#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define DIMENSION 10 /* Conway game of life field dimension. */
#define CALC_DIMENSION 3 /* Dimension for matrix which holds cell and neighbourg information. */

int PRIORITIES_ARRAY[DIMENSION*DIMENSION]; /* Array of priorities for threads. */
int CELL_DONE_MATRIX[DIMENSION][DIMENSION]; /* Matrix to indicate which cell completed evolution for current generation. */
int CELLS_MATRIX[DIMENSION][DIMENSION]; /* Conway game of life main matrix. */
int PRIORITY_IDX = 0; /* Index to next priority value to decide next cell (thread) for evolution. */

static sem_t displaySem; /* Semaphore for main thread, which controls displaying of matrix and starts evolution. */
static sem_t calcSem; /* Semaphore to control critical section of cells matrix access and evolution. */

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
               printf("%c " , mat[i][j] == 1 ? 'o' : ' ');
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

/* qsort compare function */
int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

/* Converts calculated priorities from matrix to an array compatible for iterating and control in thread function. */
void priorities_to_array(int prior_mat[][DIMENSION]){
    //initialize
    int k=0;
    for(int i=0;i<DIMENSION;i++){
        for(int j=0;j<DIMENSION;j++){
            PRIORITIES_ARRAY[k] = prior_mat[i][j];
            k++;
        }
    }
    //sort
    qsort(PRIORITIES_ARRAY,DIMENSION*DIMENSION,sizeof(int), cmpfunc);
}

/* After evolution of one generation is over, function resets cells done matrix for next cycle. */
void reset_done_matrix(){
    for(int i=0;i<DIMENSION;i++){
        for(int j=0;j<DIMENSION;j++){
            CELL_DONE_MATRIX[i][j] = 0;
        }
    }
}

/* Thread function, each thread is responsible for one cell. In every cycle it creates matrix for calculate_conway 
   and inserts new state in cells matrix for it's responsible cell. */
void* cell_thread_fun(void* param){
     int* pair = (int*) param;
     int cells_scope[CALC_DIMENSION][CALC_DIMENSION];
     while(1){
          sem_wait(&calcSem);
          if(!CELL_DONE_MATRIX[pair[0]][pair[1]] && (pair[2] == PRIORITIES_ARRAY[PRIORITY_IDX])){
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
               }
               CELLS_MATRIX[pair[0]][pair[1]] = calculate_conway(cells_scope);
               PRIORITY_IDX++;
               CELL_DONE_MATRIX[pair[0]][pair[1]] = 1;
               if((pair[0] == (DIMENSION - 1)) && (pair[1] == (DIMENSION - 1)))
                    sem_post(&displaySem);
               else
                    sem_post(&calcSem);
          }
          else{
               sem_post(&calcSem);
               usleep(1000);
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
     if(argc != 2){
          printf("Missing command line args !\n");
          return -1;
     }
     int sleep_time = atoi(argv[1]);
     sem_init(&displaySem,0,1);
     sem_init(&calcSem,0,0);
     int pairs[DIMENSION*DIMENSION][3];
     pthread_t cell_threads[DIMENSION][DIMENSION];
     int priorities[DIMENSION][DIMENSION];
     calculate_priorities(priorities);
     priorities_to_array(priorities);
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
     while(1){ 
          sem_wait(&displaySem);

          reset_done_matrix();
          PRIORITY_IDX = 0;
          system("clear");
          print_matrix(CELLS_MATRIX);
          sem_post(&calcSem);
          
          sleep(sleep_time);
     }
     return 0;
}
