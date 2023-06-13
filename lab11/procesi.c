#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <sched.h>

static struct timespec r;
static int period;


#define NSEC_PER_SEC 1000000000ULL


static inline void timespec_add_us(struct timespec *t, uint64_t d)
{
    d *= 1000;
    d += t->tv_nsec;
    while (d >= NSEC_PER_SEC) {
        d -= NSEC_PER_SEC;
		    t->tv_sec += 1;
    }
    t->tv_nsec = d;
}

/* funkcija za inicijalizaciju tajmera */
int start_periodic_timer(uint64_t offs, int t)
{
    clock_gettime(CLOCK_REALTIME, &r);
    timespec_add_us(&r, offs);

    period = t;

    return 0;
}


/* funkcija koja racuna vrijeme sljedeceg budjenja */
static void wait_next_activation(void)
{
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &r, NULL);

    timespec_add_us(&r, period);

    //printf("*** Task 1 wait to ... %ld[%ld][%ld]\n", r1.tv_nsec / 1000000, r1.tv_nsec, r1.tv_sec);

}



// periodicni zadatak na 40 milisekundi
void task1(void)
{	
	  int i,j;
	 
	  for (i=0; i<4; i++) {
		for (j=0; j<1000; j++) ;
		printf("1");
		fflush(stdout);
	  }
}

// periodicni zadatak na 80 milisekundi
void task2(void)
{
	  int i,j;

	  for (i=0; i<6; i++) {
		for (j=0; j<10000; j++) ;
		printf("2");
		fflush(stdout);
	  }
	  
}

// periodicni zadatak na 120 milisekundi
void task3(void)
{
	  int i,j;

	  for (i=0; i<6; i++) {
		for (j=0; j<100000; j++) ;
		printf("3");
		fflush(stdout);
	  }
}


int main(int argc, char *argv[])
{
    
    pid_t task_proces1;
    pid_t task_proces2;
    pid_t task_proces3;
    
    int status1, status2, status3;
    
    /* kreiraju se tri procesa */
    task_proces1 = fork();
    task_proces2 = fork();
    task_proces2 = fork();
    
    if(task_proces1 < 0 || task_proces2 < 0 || task_proces3 < 0){
    	perror("Fork");	
    	return -1;
    }
    
    
    if(task_proces1 == 0){
    	
    	// inicijalizacija zadatka i rasporedjivanja
    	int pmin = sched_get_priority_min(SCHED_FIFO);
    	struct sched_param param;
    	param.sched_priority = pmin;
    	sched_setscheduler(0, SCHED_FIFO, &param);
    	
    	// inicijalizacija tajmera
    	int res;
    	res = start_periodic_timer(0, 40000);
    	if(res < 0){
    		perror("Start periodic timer error!");
    		return -1;
    	}
    	
    	while(1){
    		wait_next_activation();
    		task1();
    	}
    	
    	return 11;
    }
    
    if(task_proces2 == 0){
    
    	// inicijalizacija zadatka i rasporedjivanja
    	int pmin = sched_get_priority_min(SCHED_FIFO);
    	struct sched_param param;
    	param.sched_priority = pmin + 10;
    	sched_setscheduler(0, SCHED_FIFO, &param);
    	
    	// inicijalizacija tajmera
    	int res;
    	res = start_periodic_timer(0, 80000);
    	if(res < 0){
    		perror("Start periodic timer error!");
    		return -1;
    	}
    	
    	while(1){
    		wait_next_activation();
    		task2();
    	}
    	
    	return 22;
    }
    
    if(task_proces3 == 0){
    
    	// inicijalizacija zadatka i rasporedjivanja
    	int pmin = sched_get_priority_min(SCHED_FIFO);
    	struct sched_param param;
    	param.sched_priority = pmin + 20;
    	sched_setscheduler(0, SCHED_FIFO, &param);
    	
    	// inicijalizacija tajmera
    	int res;
    	res = start_periodic_timer(0, 120000);
    	if(res < 0){
    		perror("Start periodic timer error!");
    		return -1;
    	}
    	
    	while(1){
    		wait_next_activation();
    		task3();
    	}
    	
    	return 33;
    }
    
    
    if(waitpid(task_proces1, &status1, 0) == -1){
    	perror("Waitpid task1 failed");
    	return -1;
    }
    
    if(waitpid(task_proces2, &status2, 0) == -1){
    	perror("Waitpid task2 failed");
    	return -1;
    }
    
    if(waitpid(task_proces3, &status3, 0) == -1){
    	perror("Waitpid task3 failed");
    	return -1;
    }
    	
    	
    return 0;
}
    





